#  Copyright 2018 Rice University                                           
#                                                                           
#  Licensed under the Apache License, Version 2.0 (the "License");          
#  you may not use this file except in compliance with the License.         
#  You may obtain a copy of the License at                                  
#                                                                           
#      http://www.apache.org/licenses/LICENSE-2.0                           
#                                                                           
#  Unless required by applicable law or agreed to in writing, software      
#  distributed under the License is distributed on an "AS IS" BASIS,        
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
#  See the License for the specific language governing permissions and      
#  limitations under the License.                                           
#  ======================================================================== 
#!python
#to run pseudo cluster on single node


import subprocess
import time
import sys

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

threadNum = "1"
sharedMemorySize = "2048"

if(len(sys.argv)==3) :
    threadNum = sys.argv[1]
    sharedMemorySize = sys.argv[2]
else:
    print bcolors.OKBLUE + "Usage: python scripts/startPseduoCluster.py numThreads (default: 1) sizeOfSharedMemoryPool (default: 2048 MB)"


print("#################################")
print("RUN A PSEDO CLUSTER")
print("#################################")

try:
    #run bin/pdb-cluster
    print bcolors.OKBLUE + "start a pdbServer as the coordinator" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-cluster', 'localhost', '8108', 'Y'])
    print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
    time.sleep(9)
    #strip shared libraries
    print bcolors.OKBLUE + "strip shared libraries" + bcolors.ENDC
    subprocess.call(['bash', 'strip libraries/*.so'])

    #run bin/pdb-server for worker
    num = 0;
    with open('conf/serverlist.test') as f:
        for each_line in f:
            print bcolors.OKBLUE + "start a pdbServer at " + each_line + "as " + str(num) + "-th worker" + bcolors.ENDC
            num = num + 1
            serverProcess = subprocess.Popen(['bin/pdb-server', threadNum, sharedMemorySize, 'localhost:8108', each_line])
            print bcolors.OKBLUE + "waiting for 9 seconds for server to be fully started..." + bcolors.ENDC
            time.sleep(9)


except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in starting peudo cluster" + bcolors.ENDC
    print e.returncode





                                              
