import ctypes
import logging
import os
import shutil
import argparse
import winsound
import time
from collections import namedtuple

DriveInfo = namedtuple('DriveInfo', 'drive_letter volume_name')

log = logging.getLogger(__name__)
logging.basicConfig()
logging.getLogger().setLevel(logging.DEBUG)

class DetectsLPCBoard(object):

    def __init__(self):
        self.kernel32 = ctypes.windll.kernel32

    def iter_drive_letters(self):
        drive_bitmask = self.kernel32.GetLogicalDrives()
        for i in range(26):
            if ((1 << i) & drive_bitmask) != 0:
                yield '%s:\\' % (chr(ord('A') + i),)

    def get_volume_name(self, drive_letter):
        vol_name_buf = ctypes.create_unicode_buffer(1024)
        result = self.kernel32.GetVolumeInformationW(
            ctypes.c_wchar_p(drive_letter),
            vol_name_buf,
            ctypes.sizeof(vol_name_buf),
            None,
            None,
            None,
            None,
            0)
        if not result:
            raise RuntimeError('Failed to retrieve drive letters')

        return vol_name_buf.value

    def detect_lpc_board(self):
        log.info('Retrieving board information')
        candidates = []
        for dl in self.iter_drive_letters():
            try:
                vol_name = self.get_volume_name(dl)
                if vol_name.startswith('CRP'):
                    candidates.append(DriveInfo(dl, vol_name))
            except RuntimeError:
                pass

        if not candidates:
            raise RuntimeError('No LPC boards detected')

        if len(candidates) > 1:
            raise RuntimeError('Multiple LPC boards detected: %s' % (candidates,))

        return candidates[0]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('bin_file')

    try:
        args = parser.parse_args()
        detect_lpc = DetectsLPCBoard()
        log.info('Detecting LPC boards')

        drive_info = detect_lpc.detect_lpc_board()
        log.info('LPC board detected: %s', drive_info)
        existing_firmware = os.path.join(drive_info.drive_letter, 'firmware.bin')

        if os.path.isfile(existing_firmware):
            log.info('Removing original image')
            os.remove(existing_firmware)

        log.info('Programming the board')
        shutil.copy(args.bin_file, existing_firmware)

        log.info('Done')
        winsound.Beep(800, 600)
    except Exception:
        for n in range(3):
            winsound.Beep(1000, 300)
            time.sleep(0.05)
        raise

if __name__ == '__main__':
    main()