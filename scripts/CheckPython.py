import os
import subprocess
import sys

REQUIREMENTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'requirements.txt')

def ValidatePackages():
    # Installs what requirements.txt lists into the Python running this script, which may be
    # a virtual environment. pip skips what is already installed.
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', '--quiet', '-r', REQUIREMENTS])