import subprocess

Import("env")

def before_build(source, target, env):
    result = subprocess.run(["python3", "src/webpage.py"], capture_output=True, text=True)
    if result.returncode != 0:
        print("Error generating header file:")
        print(result.stderr)
        env.Exit(1)
    else:
        print(result.stdout)

env.AddPreAction("${BUILD_DIR}/src/main.cpp.o", before_build)
