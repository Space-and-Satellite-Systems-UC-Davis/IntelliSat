"""Build entry must require deliberate image inputs and the specified compiler."""
from pathlib import Path
import os
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'onboard/build_provisioning.sh'


class BuildInputTests(unittest.TestCase):
    def test_missing_image_arguments_fails_before_creating_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / 'output'
            env = dict(os.environ, BUILD_DIR=str(output), ARM_GCC='/not/a/compiler')
            env.pop('PROVISION_IMAGE_ARGS', None)
            run = subprocess.run(['sh', str(SCRIPT)], env=env, capture_output=True,
                                 text=True, timeout=5)
            self.assertEqual(run.returncode, 2)
            self.assertIn('explicit --boot-count', run.stderr)
            self.assertFalse(output.exists())

    def test_wrong_compiler_version_fails_before_creating_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            compiler = Path(directory) / 'compiler'
            compiler.write_text('#!/bin/sh\nprintf "arm-none-eabi-gcc 14.3.1\\n"\n')
            compiler.chmod(0o700)
            output = Path(directory) / 'output'
            env = dict(os.environ, BUILD_DIR=str(output), ARM_GCC=str(compiler),
                       PROVISION_IMAGE_ARGS='--boot-count 0 --adcs-mask 0')
            run = subprocess.run(['sh', str(SCRIPT)], env=env, capture_output=True,
                                 text=True, timeout=5)
            self.assertEqual(run.returncode, 1)
            self.assertIn('requires GNU Arm 13.3.1', run.stderr)
            self.assertFalse(output.exists())


if __name__ == '__main__':
    unittest.main()
