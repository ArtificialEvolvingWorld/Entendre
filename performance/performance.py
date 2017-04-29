#!/opt/software/anaconda3/bin/python

import subprocess
if __name__=="__main__":
    for num_networks in [pow(2,n) for n in range(10,21)]:
        completed = subprocess.run(
            ['../bin/performance_tests', '-n',str(num_networks)],
            stdout=subprocess.PIPE,
        )
        print(completed.stdout.decode('utf-8'))
