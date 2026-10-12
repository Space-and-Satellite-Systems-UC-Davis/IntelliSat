"""Run the compiled SPI function against a small, controlled register model."""
import sys
import unittest
from pathlib import Path

from elftools.elf.elffile import ELFFile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_HOOK_CODE
from unicorn import UC_HOOK_MEM_READ, UC_HOOK_MEM_WRITE, UC_MEM_READ
from unicorn.arm_const import (
    UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3,
    UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC,
)

FLASH, RAM, SPI = 0x08000000, 0x20000000, 0x40003800
TX, RX, RETURN = RAM + 0x100, RAM + 0x200, FLASH + 0xF000
TXE, RXNE = 2, 1


class Transfer:
    def __init__(self, elf, *, stall=None, delay=0, extra_rx=False):
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
        self.uc.mem_map(FLASH, 0x10000)
        self.uc.mem_map(RAM, 0x10000)
        self.uc.mem_map(SPI & ~0xFFF, 0x1000)
        with Path(elf).open("rb") as stream:
            image = ELFFile(stream)
            for segment in image.iter_segments():
                if segment["p_type"] == "PT_LOAD" and segment["p_filesz"]:
                    self.uc.mem_write(segment["p_vaddr"], segment.data())
            symbols = image.get_section_by_name(".symtab")
            self.entry = symbols.get_symbol_by_name("spi_transmitReceive")[0]["st_value"]
            clock = symbols.get_symbol_by_name("getSysTime")[0]["st_value"] & ~1
        self.stall, self.delay, self.extra_rx = stall, delay, extra_rx
        self.ticks = 0
        self.writes, self.reads = [], []
        self.pending = None
        self.ready_at = 0
        self.uc.hook_add(UC_HOOK_CODE, self.clock, begin=clock, end=clock)
        self.uc.hook_add(UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE,
                         self.register, begin=SPI, end=SPI + 0x1F)

    def clock(self, uc, address, size, _):
        # Time advances on each clock read, including reads in polling loops.
        uc.reg_write(UC_ARM_REG_R0, self.ticks & 0xFFFFFFFF)
        uc.reg_write(UC_ARM_REG_R1, self.ticks >> 32)
        self.ticks += 1
        uc.reg_write(UC_ARM_REG_PC, uc.reg_read(UC_ARM_REG_LR))

    def register(self, uc, access, address, size, value, _):
        if address == SPI + 8 and access == UC_MEM_READ:
            blocked_tx = (self.stall == "before_tx" or
                          (self.stall == "after_tx" and self.writes) or
                          (self.stall == "second_tx" and self.reads))
            blocked_rx = self.stall == "rx" or (
                self.stall == "second_rx" and len(self.writes) >= 2)
            ready = self.pending is not None and self.ticks >= self.ready_at
            status = (0 if blocked_tx else TXE) | (
                RXNE if ready and not blocked_rx else 0)
            uc.mem_write(address, status.to_bytes(size, "little"))
        elif address == SPI + 12:
            if access == UC_MEM_READ:
                self.reads.append((size, self.ticks))
                uc.mem_write(address, (self.pending or 0).to_bytes(size, "little"))
                if not self.extra_rx:
                    self.pending = None
            else:
                self.writes.append((value, size, self.ticks))
                self.pending = 0x80 + len(self.writes)
                self.ready_at = self.ticks + self.delay
        else:
            raise AssertionError(f"Unexpected SPI register access at {address:#x}")

    def run(self, size=3, *, tx=True, rx=True):
        self.uc.mem_write(TX, bytes(range(0x11, 0x11 + size)))
        self.uc.mem_write(RX - 1, b"\xcc" * (size + 2))
        for reg, value in [(UC_ARM_REG_R0, SPI), (UC_ARM_REG_R1, TX if tx else 0),
                           (UC_ARM_REG_R2, RX if rx else 0), (UC_ARM_REG_R3, size),
                           (UC_ARM_REG_SP, RAM + 0xF000), (UC_ARM_REG_LR, RETURN | 1)]:
            self.uc.reg_write(reg, value)
        self.uc.mem_write(RAM + 0xF000, b"\0" * 4)  # fifth argument: dma=false
        self.uc.emu_start(self.entry | 1, RETURN, count=100000)
        if self.uc.reg_read(UC_ARM_REG_PC) != RETURN:
            raise AssertionError("SPI function did not return within instruction limit")
        self.result = self.uc.reg_read(UC_ARM_REG_R0)
        self.buffer = bytes(self.uc.mem_read(RX, size))
        assert self.uc.mem_read(RX - 1, 1) == b"\xcc", "wrote before receive buffer"
        assert self.uc.mem_read(RX + size, 1) == b"\xcc", "wrote past receive buffer"
        return self


class SpiTests(unittest.TestCase):
    def transfer(self, **kwargs):
        return Transfer(ELF, **kwargs)

    def test_full_duplex(self):
        transfer = self.transfer().run()
        self.assertEqual(transfer.result, 1)
        self.assertEqual([v for v, _, _ in transfer.writes], [0x11, 0x12, 0x13])
        self.assertEqual(transfer.buffer, b"\x81\x82\x83")
        self.assertTrue(all(size == 1 for _, size, _ in transfer.writes))
        self.assertEqual([size for size, _ in transfer.reads], [1, 1, 1])

    def test_null_buffers(self):
        for tx, rx in [(False, True), (True, False), (False, False)]:
            with self.subTest(tx=tx, rx=rx):
                transfer = self.transfer().run(tx=tx, rx=rx)
                self.assertEqual(transfer.result, 1)
                expected = [0x11, 0x12, 0x13] if tx else [0xAA] * 3
                self.assertEqual([v for v, _, _ in transfer.writes], expected)
                self.assertEqual(len(transfer.reads), 3)
                self.assertEqual(transfer.buffer, b"\x81\x82\x83" if rx else b"\xcc" * 3)

    def test_zero_length(self):
        transfer = self.transfer().run(size=0, tx=False, rx=False)
        self.assertEqual(transfer.result, 1)
        self.assertEqual(transfer.writes, [])
        self.assertEqual(transfer.reads, [])

    def test_timeout_before_write(self):
        transfer = self.transfer(stall="before_tx").run()
        self.assertEqual(transfer.result, 0)
        self.assertEqual(transfer.writes, [])
        self.assertEqual(transfer.reads, [])
        self.assertEqual(transfer.buffer, b"\xcc" * 3)

    def test_timeout_after_write(self):
        transfer = self.transfer(stall="after_tx").run()
        self.assertEqual(transfer.result, 0)
        self.assertEqual(len(transfer.writes), 1)
        self.assertEqual(transfer.reads, [])
        self.assertEqual(transfer.buffer, b"\xcc" * 3)

    def test_receive_timeout(self):
        transfer = self.transfer(stall="rx").run()
        self.assertEqual(transfer.result, 0)
        self.assertEqual(len(transfer.writes), 1)
        self.assertEqual(transfer.reads, [])
        self.assertEqual(transfer.buffer, b"\xcc" * 3)

    def test_partial_transfer(self):
        for stall, writes in [("second_tx", 1), ("second_rx", 2)]:
            with self.subTest(stall=stall):
                transfer = self.transfer(stall=stall).run()
                self.assertEqual(transfer.result, 0)
                self.assertEqual(len(transfer.writes), writes)
                self.assertEqual(len(transfer.reads), 1)
                self.assertEqual(transfer.buffer, b"\x81\xcc\xcc")

    def test_one_deadline_for_all_bytes(self):
        transfer = self.transfer(delay=30).run(size=2)
        self.assertEqual(transfer.result, 0)
        self.assertEqual(transfer.buffer, b"\x81\xcc")
        self.assertEqual(len(transfer.writes), 2)
        self.assertEqual(transfer.ticks, 51)  # clock starts at 0; deadline is 50 ms

    def test_delay_within_deadline(self):
        transfer = self.transfer(delay=10).run()
        self.assertEqual(transfer.result, 1)
        self.assertEqual(transfer.buffer, b"\x81\x82\x83")

    def test_receive_count_is_bounded(self):
        transfer = self.transfer(extra_rx=True).run(size=1)
        self.assertEqual(transfer.result, 1)
        self.assertEqual(len(transfer.reads), 1)
        self.assertEqual(transfer.buffer, b"\x81")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit(f"Usage: {sys.argv[0]} path/to/spi.elf")
    ELF = sys.argv.pop()
    unittest.main(verbosity=2)
