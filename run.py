from concurrent import futures
import os
import subprocess
import time

def fuzzy_runner(id_process):
    fuzz = subprocess.Popen(["./build/fuzzytest", "-s 100", "-i {}".format(id_process), "-v 0"], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
    for line in fuzz.stdout:
        print(line)

if __name__ == "__main__":
    number_of_processes = 1 #os.cpu_count()

    begin = time.time()
    with futures.ProcessPoolExecutor() as executor:
        res = executor.map(fuzzy_runner, range(number_of_processes))
        list(res)
    print(time.time() - begin)
        