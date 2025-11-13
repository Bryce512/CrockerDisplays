import shutil
import os
import glob

# Get the project directory - look for .pio folder to determine root
cwd = os.getcwd()

# Build paths
src = os.path.join(cwd, "include", "lv_conf.h")

# Find the LVGL directory in libdeps
lvgl_dirs = glob.glob(os.path.join(cwd, ".pio", "libdeps", "*", "lvgl", "src"))

if lvgl_dirs:
    dst = os.path.join(lvgl_dirs[0], "lv_conf.h")
    
    # Create destination directory if needed
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    
    # Copy the file
    if os.path.exists(src):
        shutil.copy2(src, dst)
        print(f"✓ Copied lv_conf.h to LVGL source")
    else:
        print(f"✗ Error: {src} not found")
else:
    print(f"✗ Error: Could not find LVGL in .pio/libdeps")