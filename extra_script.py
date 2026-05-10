print("====Before_Build====")
Import("env")
import gzip
import shutil
import os
import subprocess
import binascii
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
import pip


# path configuration
web_ui_source_path = "./webui/old/"
web_ui_target_path = "./src/webui/"
web_ui_source_files = ["recoverymode.html", "restart.html"]

def install_package(package):
    import sys
    subprocess.call([sys.executable, "-m", "pip", "install", "--upgrade", package])

install_package("html_utils_becothal")	
from html_utils import HTML

def convert_web_ui_to_include_files():
    print("---- Convert Web UI to include files ----")

    # compress html files and create a uint8_t array
    for web_ui_file in web_ui_source_files:
        web_ui_file_header_array = web_ui_file + ".gz"

        html_file = HTML()
        html_file.read_file(web_ui_source_path + web_ui_file)
        html_file.inline_css()
        html_file.inline_js()
        html_file.remove_comments("", "<!--", "-->")
        html_file.images_to_base64()
        
        html_file_gzip = gzip.compress(html_file.to_string().encode("utf-8"))
            
        html_file_gzip_hex = binascii.hexlify(html_file_gzip).decode("UTF-8").upper()
        html_file_gzip_hex_array = ["0x" + html_file_gzip_hex[i:i + 2] + ", " for i in range(0, len(html_file_gzip_hex), 2)]
        char_array_string = "const uint8_t " + web_ui_file_header_array.replace(".", "_") + "[] = {" + str("").join(html_file_gzip_hex_array) + "};"
        
        with open(web_ui_target_path + web_ui_file + ".gz.h", 'w') as f:
            f.write(char_array_string)

convert_web_ui_to_include_files()



# --- ESPRandom Patch: library missing #include <vector> (bug in protohaus/ESPRandom@1.4.1) ---
def patch_esp_random():
    libdeps_base = env.subst("$PROJECT_LIBDEPS_DIR")
    pioenv = env.subst("$PIOENV")
    esp_random_h = os.path.join(libdeps_base, pioenv, "ESPRandom", "ESPRandom.h")

    if not os.path.exists(esp_random_h):
        print("[ESPRandom Patch] not found, skipping")
        return

    with open(esp_random_h, "r", encoding="utf-8") as f:
        content = f.read()

    if "#include <vector>" in content:
        print("[ESPRandom Patch] already applied, skipping")
        return

    with open(esp_random_h, "w", encoding="utf-8") as f:
        f.write("#include <vector>\n" + content)
    print("[ESPRandom Patch] applied to: " + esp_random_h)

patch_esp_random()
