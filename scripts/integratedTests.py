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
#JiaNote: to run integrated tests on standalone node for storing and querying data


import subprocess
import time
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

print("#################################")
print("CLEAN THE TESTING ENVIRONMENT")
print("#################################")
subprocess.call(['bash', './scripts/cleanupNode.sh'])
numTotal = 2
numErrors = 0
numPassed = 0


print("#################################")
print("RUN STANDALONE INTEGRATION TESTS")
print("#################################")

try:
    #run bin/pdb-server
    print bcolors.OKBLUE + "start a standalone pdbServer" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512'])
    print bcolors.OKBLUE + "to check whether server has been fully started..." + bcolors.ENDC
    subprocess.call(['bash', './scripts/checkProcess.sh', 'pdb-server'])
    print bcolors.OKBLUE + "to sleep to wait for server to be fully started" + bcolors.ENDC
    time.sleep(30)
    #run bin/test46 1024
    print bcolors.OKBLUE + "start a data client to store data to pdbServer" + bcolors.ENDC
    subprocess.check_call(['bin/test46', '640'])
    #run bin/test44 in a loop for 10 times
    print bcolors.OKBLUE + "start a query client to query data from pdbServer" + bcolors.ENDC
    subprocess.check_call(['bin/test44', 'n'])

except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in running standalone integration tests" + bcolors.ENDC
    print e.returncode
    numErrors = numErrors + 1

else:
    print bcolors.OKBLUE + "[PASSED] standalone integration tests" + bcolors.ENDC
    numPassed = numPassed + 1



subprocess.call(['bash', './scripts/cleanupNode.sh'])
print bcolors.OKBLUE + "waiting for 20 seconds for server to be fully cleaned up..."
time.sleep(20)

print("#################################")
print("RUN DISTRIBUTED INTEGRATION TESTS")
print("#################################")

try:
    #run bin/pdb-cluster
    print bcolors.OKBLUE + "start a pdbServer as the coordinator" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-cluster', 'localhost', '8108', 'Y'])
    print bcolors.OKBLUE + "to check whether server is started..." + bcolors.ENDC
    subprocess.call(['bash', './scripts/checkProcess.sh', 'pdb-cluster'])
    print bcolors.OKBLUE + "to sleep to wait for server to be fully started" + bcolors.ENDC
    time.sleep(9)

    #run bin/pdb-server for instance 1
    print bcolors.OKBLUE + "start a pdbServer as the 1st worker" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8109'])
    print bcolors.OKBLUE + "waiting for 15 seconds for server to be fully started..." + bcolors.ENDC
    time.sleep(15)
    subprocess.check_call(['bin/CatalogTests',  '--port', '8108', '--serverAddress', 'localhost', '--command', 'register-node', '--node-ip', 'localhost', '--node-port',  '8109', '--node-name', 'worker', '--node-type', 'worker'])

    #run bin/pdb-server for instance 2
    print bcolors.OKBLUE + "start a pdbServer as the 2nd worker" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8110'])
    print bcolors.OKBLUE + "waiting for 15 seconds for server to be fully started..." + bcolors.ENDC
    time.sleep(15)
    subprocess.check_call(['bin/CatalogTests',  '--port', '8108', '--serverAddress', 'localhost', '--command', 'register-node', '--node-ip', 'localhost', '--node-port',  '8110', '--node-name', 'worker', '--node-type', 'worker'])

    #run bin/pdb-server for instance 3
    print bcolors.OKBLUE + "start a pdbServer as the 3rd worker" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8111'])
    print bcolors.OKBLUE + "waiting for 15 seconds for server to be fully started..." + bcolors.ENDC
    time.sleep(15)
    subprocess.check_call(['bin/CatalogTests',  '--port', '8108', '--serverAddress', 'localhost', '--command', 'register-node', '--node-ip', 'localhost', '--node-port',  '8111', '--node-name', 'worker', '--node-type', 'worker'])

    #run bin/pdb-server for instance 4
    print bcolors.OKBLUE + "start a pdbServer as the 4th worker" + bcolors.ENDC
    serverProcess = subprocess.Popen(['bin/pdb-server', '1', '512', 'localhost:8108', 'localhost:8112'])
    print bcolors.OKBLUE + "waiting for 15 seconds for server to be fully started..." + bcolors.ENDC
    time.sleep(15)
    subprocess.check_call(['bin/CatalogTests',  '--port', '8108', '--serverAddress', 'localhost', '--command', 'register-node', '--node-ip', 'localhost', '--node-port',  '8112', '--node-name', 'worker', '--node-type', 'worker'])

    #run bin/test52
    print bcolors.OKBLUE + "start a query client to store and query data from pdb cluster" + bcolors.ENDC
    subprocess.check_call(['bin/test52', 'N', 'Y', '1024', 'localhost'])

except subprocess.CalledProcessError as e:
    print bcolors.FAIL + "[ERROR] in running distributed integration tests" + bcolors.ENDC
    print e.returncode
    numErrors = numErrors + 1

else:
    print bcolors.OKBLUE + "[PASSED] distributed integration tests" + bcolors.ENDC
    numPassed = numPassed + 1


#subprocess.call(['bash', './scripts/cleanupNode.sh'])


print("#################################")
print("SUMMRY")
print("#################################")
print bcolors.OKGREEN + "TOTAL TESTS: " + str(numTotal) + bcolors.ENDC
print bcolors.OKGREEN + "PASSED TESTS: " + str(numPassed) + bcolors.ENDC
print bcolors.FAIL + "FAILED TESTS: " + str(numErrors) + bcolors.ENDC
                                              
