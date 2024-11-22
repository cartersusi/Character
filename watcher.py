import os
import time
import subprocess
import signal

def watch_file(file_path, compile_command, run_command):
    last_size = None
    process = None

    try:
        while True:
            # Check file size
            current_size = os.path.getsize(file_path)
            if last_size is None:
                last_size = current_size

            # If file size changes, recompile and restart program
            if current_size != last_size:
                print("File change detected, recompiling...")
                last_size = current_size
                
                # Stop currently running process
                if process:
                    print("Stopping running program...")
                    process.terminate()
                    process.wait()

                # Compile the program
                compile_result = subprocess.run(compile_command, shell=True, text=True)
                if compile_result.returncode != 0:
                    print("Compilation failed, watching for further changes...")
                    time.sleep(1)
                    continue

                # Run the program
                print("Running the compiled program...")
                process = subprocess.Popen(run_command, shell=True, preexec_fn=os.setsid)

            time.sleep(1)  # Watch for changes every second

    except KeyboardInterrupt:
        print("Exiting...")
        if process:
            os.killpg(os.getpgid(process.pid), signal.SIGTERM)

if __name__ == "__main__":
    main_file = "render.cpp"
    compile_cmd = "make"
    run_cmd = "./character"

    #compile_result = subprocess.run(compile_cmd, shell=True, text=True)
    #if compile_result.returncode != 0:
    #    print("Compilation failed, watching for further changes...")
    #    exit(1)
    #process = subprocess.Popen(run_cmd, shell=True, preexec_fn=os.setsid)

    if not os.path.exists(main_file):
        print(f"Error: {main_file} does not exist.")
    else:
        watch_file(main_file, compile_cmd, run_cmd)