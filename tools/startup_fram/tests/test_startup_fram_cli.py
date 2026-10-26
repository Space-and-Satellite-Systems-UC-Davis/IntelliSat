"""End-to-end tests for the startup_fram.py command line (generate / inspect)."""

import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

LANE_ROOT = pathlib.Path(__file__).resolve().parents[3]
TOOL = LANE_ROOT / "tools" / "startup_fram" / "startup_fram.py"
sys.path.insert(0, str(LANE_ROOT / "tools" / "startup_fram"))
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import contract_fixture as fixture  # noqa: E402
import startup_image as si  # noqa: E402


def run_tool(*args: str) -> subprocess.CompletedProcess:
    return subprocess.run(
        [sys.executable, str(TOOL), *args],
        capture_output=True, text=True, timeout=120, check=False,
    )


class CliTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.tmp = pathlib.Path(self._tmp.name)
        self.addCleanup(self._tmp.cleanup)

    def generate(self, *args: str) -> subprocess.CompletedProcess:
        return run_tool("generate", *args)

    def test_generate_binary_and_header_match_the_codec(self):
        binary = self.tmp / "startup.bin"
        header = self.tmp / "startup_image.h"
        result = self.generate("--boot-count", "4", "--adcs-mask", "0x00100001",
                              "--binary", str(binary), "--header", str(header))
        self.assertEqual(result.returncode, 0, result.stderr)
        image = binary.read_bytes()
        self.assertEqual(image, si.fresh_image(4, 0x00100001))
        self.assertEqual(len(image), 48)
        text = header.read_text()
        self.assertIn("static const uint8_t STARTUP_FRAM_IMAGE[STARTUP_FRAM_IMAGE_SIZE]", text)
        self.assertIn("#define STARTUP_FRAM_IMAGE_ARMED 0u", text)
        self.assertIn("STARTUP_FRAM_IMAGE_BOOT_COUNT 0x00000004u", text)
        self.assertIn("STARTUP_FRAM_IMAGE_ADCS_MASK 0x00100001u", text)
        # every image byte appears exactly once, in order
        listed = [token.strip() for token in text.split("{", 1)[1].split("}", 1)[0].replace(
            ",", " ").split()]
        self.assertEqual([int(token, 16) for token in listed], list(image))

    def test_generate_accepts_sensor_names(self):
        binary = self.tmp / "named.bin"
        result = self.generate("--boot-count", "0", "--adcs-sensors", "Sun0,Mag1,Hdd1",
                              "--binary", str(binary))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(si.image_adcs_mask(binary.read_bytes()),
                         0b1 | (1 << 18) | (1 << 20))

    def test_generate_is_unarmed_unless_asked(self):
        plain = self.tmp / "plain.bin"
        armed = self.tmp / "armed.bin"
        self.assertEqual(self.generate("--boot-count", "0", "--adcs-mask", "0",
                                       "--binary", str(plain)).returncode, 0)
        result = self.generate("--boot-count", "0", "--adcs-mask", "0", "--armed",
                              "--binary", str(armed))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("ARMED", result.stdout)
        self.assertFalse(si.validate_image(plain.read_bytes()).armed)
        self.assertTrue(si.validate_image(armed.read_bytes()).armed)

    def test_generate_is_exclusive_by_default(self):
        binary = self.tmp / "keep.bin"
        first = self.generate("--boot-count", "0", "--adcs-mask", "0",
                              "--binary", str(binary))
        self.assertEqual(first.returncode, 0, first.stderr)
        original = binary.read_bytes()

        second = self.generate("--boot-count", "1", "--adcs-mask", "0",
                               "--binary", str(binary))
        self.assertEqual(second.returncode, 1)
        self.assertIn("refusing to overwrite", second.stderr)
        self.assertEqual(binary.read_bytes(), original)

        third = self.generate("--boot-count", "1", "--adcs-mask", "0",
                              "--binary", str(binary), "--overwrite")
        self.assertEqual(third.returncode, 0, third.stderr)
        self.assertEqual(binary.read_bytes(), si.fresh_image(1, 0))

    def test_generate_refuses_before_writing_any_output(self):
        binary = self.tmp / "first.bin"
        header = self.tmp / "first.h"
        self.assertEqual(self.generate("--boot-count", "0", "--adcs-mask", "0",
                                       "--binary", str(binary), "--header", str(header)
                                       ).returncode, 0)

        blocked_binary = self.tmp / "second.bin"
        result = self.generate("--boot-count", "1", "--adcs-mask", "0",
                              "--binary", str(blocked_binary), "--header", str(header))
        self.assertEqual(result.returncode, 1)
        self.assertIn("no output was written", result.stderr)
        self.assertFalse(blocked_binary.exists())
        self.assertEqual(binary.read_bytes(), si.fresh_image(0, 0))

    def test_generate_requires_explicit_output_and_mask(self):
        no_output = self.generate("--boot-count", "0", "--adcs-mask", "0")
        self.assertEqual(no_output.returncode, 1)
        self.assertIn("--binary and/or --header", no_output.stderr)

        no_mask = self.generate("--boot-count", "0", "--binary", str(self.tmp / "x.bin"))
        # argparse enforces the required mutually exclusive group.
        self.assertEqual(no_mask.returncode, 2)

    def test_generate_rejects_conflicting_or_invalid_arguments(self):
        both = self.generate("--boot-count", "0", "--adcs-mask", "0",
                             "--adcs-sensors", "Sun0", "--binary", str(self.tmp / "x.bin"))
        self.assertEqual(both.returncode, 2)

        reserved = self.generate("--boot-count", "0", "--adcs-mask", "0x00200000",
                                "--binary", str(self.tmp / "r.bin"))
        self.assertEqual(reserved.returncode, 1)
        self.assertIn("reserved bits 21..31", reserved.stderr)

        saturated = self.generate("--boot-count", "0xFFFFFFFF", "--adcs-mask", "0",
                                 "--binary", str(self.tmp / "s.bin"))
        self.assertEqual(saturated.returncode, 1)
        self.assertIn("boot_count", saturated.stderr)

        unknown = self.generate("--boot-count", "0", "--adcs-sensors", "Sun12",
                               "--binary", str(self.tmp / "u.bin"))
        self.assertEqual(unknown.returncode, 1)
        self.assertIn("unknown ADCS sensor", unknown.stderr)
        self.assertIn("Sun0", unknown.stderr)

        for path in ("r.bin", "s.bin", "u.bin"):
            self.assertFalse((self.tmp / path).exists())

    def test_generate_reports_addresses_and_decoded_fields(self):
        binary = self.tmp / "report.bin"
        result = self.generate("--boot-count", "12", "--adcs-sensors", "Sun0,Coil0",
                              "--binary", str(binary))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("schema SSP1", result.stdout)
        self.assertIn("addresses 0..47", result.stdout)
        self.assertIn("boot counter    12", result.stdout)
        self.assertIn("ADCS enabled    Sun0, Coil0", result.stdout)
        self.assertIn("resume state  0 (INITIAL_CHECKS)", result.stdout)
        self.assertIn("attempts used 0/7", result.stdout)
        self.assertIn("remaining_ms  1800000", result.stdout)
        self.assertIn("commit marker 0xA5", result.stdout)
        self.assertIn(binary.read_bytes().hex(), result.stdout)

    def test_generate_is_deterministic(self):
        first = self.tmp / "a.bin"
        second = self.tmp / "b.bin"
        for path in (first, second):
            self.assertEqual(self.generate("--boot-count", "9", "--adcs-sensors", "Mag0",
                                          "--binary", str(path)).returncode, 0)
        self.assertEqual(first.read_bytes(), second.read_bytes())

    def test_generate_honours_header_symbol(self):
        header = self.tmp / "custom.h"
        result = self.generate("--boot-count", "0", "--adcs-mask", "0",
                              "--header", str(header),
                              "--header-symbol", "STARTUP_FRAM_PROVISION_IMAGE")
        self.assertEqual(result.returncode, 0, result.stderr)
        text = header.read_text()
        self.assertIn("STARTUP_FRAM_PROVISION_IMAGE[STARTUP_FRAM_PROVISION_IMAGE_SIZE]", text)
        self.assertIn("#ifndef STARTUP_FRAM_PROVISION_IMAGE_H", text)

    def test_reference_vector_roundtrips_through_the_cli(self):
        binary = self.tmp / "vector.bin"
        result = self.generate("--boot-count", "0", "--adcs-mask", "0",
                              "--binary", str(binary))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(binary.read_bytes().hex(), fixture.CONTRACT_IMAGE_HEX)

    def test_inspect_accepts_a_sequence_max_record(self):
        binary = self.tmp / "seq_max.bin"
        binary.write_bytes(bytes.fromhex(fixture.SEQ_MAX_IMAGE_HEX))
        result = run_tool("inspect", str(binary), "--json")
        self.assertEqual(result.returncode, 0, result.stderr)
        payload = json.loads(result.stdout)
        self.assertTrue(payload["valid"])
        self.assertTrue(payload["armed"])
        self.assertEqual(payload["slots"][0]["sequence"], si.SEQ_MAX)
        self.assertEqual(payload["slots"][0]["state_name"], "MAIN_OPS")
        self.assertEqual(payload["active_slot"], "B")

        table = run_tool("inspect", str(binary))
        self.assertEqual(table.returncode, 0, table.stderr)
        self.assertIn(f"sequence      {si.SEQ_MAX}", table.stdout)
        self.assertIn("WARNING: this image is ARMED", table.stdout)

    def test_inspect_valid_image(self):
        binary = self.tmp / "inspect.bin"
        self.assertEqual(self.generate("--boot-count", "2", "--adcs-mask", "0x00000003",
                                       "--binary", str(binary)).returncode, 0)
        result = run_tool("inspect", str(binary))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("format          SSP1", result.stdout)
        self.assertIn("boot counter    2", result.stdout)
        self.assertIn("00x".replace("00x", "0x00000003"), result.stdout)
        self.assertIn("ADCS enabled    Sun0, Sun1", result.stdout)
        self.assertIn("slot A          bytes 8..27", result.stdout)
        self.assertIn("slot B          bytes 28..47", result.stdout)
        self.assertIn("active slot", result.stdout)
        self.assertIn("armed           no", result.stdout)

    def test_inspect_json(self):
        binary = self.tmp / "json.bin"
        self.assertEqual(self.generate("--boot-count", "5", "--adcs-sensors", "Imu1",
                                       "--binary", str(binary)).returncode, 0)
        result = run_tool("inspect", str(binary), "--json")
        self.assertEqual(result.returncode, 0, result.stderr)
        payload = json.loads(result.stdout)
        self.assertTrue(payload["valid"])
        self.assertEqual(payload["boot_count"], 5)
        self.assertEqual(payload["adcs_mask"], "0x00010000")
        self.assertEqual(payload["adcs_enabled"], ["Imu1"])
        self.assertFalse(payload["armed"])
        self.assertEqual(len(payload["slots"]), 2)
        for slot in payload["slots"]:
            self.assertEqual(slot["sequence"], 1)
            self.assertEqual(slot["state"], 0)
            self.assertEqual(slot["state_name"], "INITIAL_CHECKS")
            self.assertEqual(slot["remaining_ms"], si.INHIBIT_MS)
        self.assertIn(payload["active_slot"], ("A", "B"))

    def test_inspect_rejects_corrupted_images(self):
        image = bytearray(si.fresh_image(0, 0))
        image[si.SLOT_A_ADDR + si.CRC_OFFSET] ^= 0xFF
        binary = self.tmp / "bad.bin"
        binary.write_bytes(bytes(image))
        result = run_tool("inspect", str(binary), "--json")
        self.assertEqual(result.returncode, 1)
        payload = json.loads(result.stdout)
        self.assertFalse(payload["valid"])
        self.assertTrue(payload["usable"])  # slot B is still a complete record
        self.assertFalse(payload["slots"][0]["valid"])
        self.assertIn("CRC32", " ".join(payload["slots"][0]["problems"]))

        both_invalid = bytearray(si.fresh_image(0, 0))
        both_invalid[si.SLOT_A_ADDR + si.MARKER_OFFSET] = si.INVALID_MARKER
        both_invalid[si.SLOT_B_ADDR + si.MARKER_OFFSET] = si.INVALID_MARKER
        binary.write_bytes(bytes(both_invalid))
        result = run_tool("inspect", str(binary))
        self.assertEqual(result.returncode, 1)
        self.assertIn("no usable Startup slot", result.stdout)

    def test_inspect_missing_and_wrong_length(self):
        missing = run_tool("inspect", str(self.tmp / "absent.bin"))
        self.assertEqual(missing.returncode, 1)
        self.assertIn("cannot read", missing.stderr)

        short = self.tmp / "short.bin"
        short.write_bytes(b"\x00" * 47)
        result = run_tool("inspect", str(short))
        self.assertEqual(result.returncode, 1)
        self.assertIn("47 bytes", result.stdout)

    def test_usage_errors_exit_two(self):
        self.assertEqual(run_tool().returncode, 2)
        self.assertEqual(run_tool("generate").returncode, 2)


if __name__ == "__main__":
    unittest.main()
