import os
import subprocess
import sys

REQUIREMENTS = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'requirements.txt')

def ValidatePackages():
    # Installs whatever requirements.txt lists that's missing, into the Python running this
    # script (a virtual environment, when it's run from one). Does nothing when it's all there.
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', '--quiet', '-r', REQUIREMENTS])