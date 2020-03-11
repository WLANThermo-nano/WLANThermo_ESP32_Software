import shutil
import os
import fnmatch
import subprocess
import zlib
from spiffsgen import *

def compress_nextion_file(target_path, source_path, filename):
    nextion_file_header_array = filename + ".zlib"
    nextion_file_uncompressed_size = os.path.getsize(source_path + filename)
    with open(source_path + filename, "rb") as f:
            nextion_file_zlib = zlib.compress(f.read())
    with open(target_path + filename + ".zlib", 'wb') as f:
            f.write(nextion_file_zlib)

data_folder = os.path.dirname(os.path.realpath(__file__)) + "\\..\\data\\"
artifacts_folder = os.path.dirname(os.path.realpath(__file__)) + "\\..\\artifacts\\"
nextion_files = fnmatch.filter(os.listdir(artifacts_folder), '*.tft')

for nextion_file in nextion_files:
    compress_nextion_file(data_folder, artifacts_folder, nextion_file)

image_size=0x3F0000
page_size=256
block_size=4096
meta_len=4
obj_name_len=32
use_magic=True
use_magic_len=True
base_dir=data_folder
output_file=artifacts_folder + "nextion_spiffs.bin"

with open(output_file, "wb") as image_file:
    spiffs_build_default = SpiffsBuildConfig(page_size, SPIFFS_PAGE_IX_LEN,
                                             block_size, SPIFFS_BLOCK_IX_LEN, meta_len,
                                             obj_name_len, SPIFFS_OBJ_ID_LEN, SPIFFS_SPAN_IX_LEN,
                                             True, True, "little",
                                             use_magic, use_magic_len)

    spiffs = SpiffsFS(image_size, spiffs_build_default)

    for root, dirs, files in os.walk(base_dir):
        for f in files:
            full_path = os.path.join(root, f)
            spiffs.create_file("/" + os.path.relpath(full_path, base_dir).replace("\\", "/"), full_path)

    image = spiffs.to_binary()

    image_file.write(image)