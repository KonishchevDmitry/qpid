#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

# Run the store tests.
# There are two sets of tests:
#  1. A subset of the normal broker python tests, dtx and persistence, but
#     run again with the SQL store loaded.
#  2. store.py, which tests recovering things across broker restarts.

$srcdir = Split-Path $myInvocation.InvocationName
$PYTHON_DIR = "$srcdir\..\..\..\python"
if (!(Test-Path $PYTHON_DIR -pathType Container)) {
    "Skipping store tests as python libs not found"
    exit 1
}

# Test runs from the tests directory but the broker executable is one level
# up, and most likely in a subdirectory from there based on what build type.
# Look around for it before trying to start it.
$subs = "Debug","Release","MinSizeRel","RelWithDebInfo"
foreach ($sub in $subs) {
  $prog = "..\$sub\qpidd.exe"
  if (Test-Path $prog) {
     break
  }
}
if (!(Test-Path $prog)) {
    "Cannot locate qpidd.exe"
    exit 1
}

# The store to test is the same build type as the broker.
$store_dir = "..\qpid\store\$sub"
if (!([string]::Compare($sub, "Debug", $True))) {
    $suffix = "d"
}

$FAILCODE = 0

# Test 1... re-run some of the regular python broker tests against a broker
# with the store module loaded.
$stamp = Get-Date -format %dMMMyyyy_HHmmss
$catalog = "broker_test_$stamp"
$cmdline = "$prog --auth=no --port=0 --log-to-file qpidd-store.log --module-dir $store_dir --catalog $catalog | foreach { set-content qpidd-store.port `$_ }"
$cmdblock = $executioncontext.invokecommand.NewScriptBlock($cmdline)
. $srcdir\background.ps1 $cmdblock

$wait_time = 0
while (!(Test-Path qpidd-store.port) -and ($wait_time -lt 10)) {
   Start-Sleep 2
   $wait_time += 2
}
if (!(Test-Path qpidd-store.port)) {
  "Time out waiting for broker to start"
  exit 1
}
set-item -path env:QPID_PORT -value (get-content -path qpidd-store.port -totalcount 1)
Remove-Item qpidd-store.port
$PYTHON_TEST_DIR = "$srcdir\..\..\..\tests\src\py\qpid_tests\broker_0_10"
$env:PYTHONPATH="$PYTHON_DIR;$PYTHON_TEST_DIR;$env:PYTHONPATH"
python $PYTHON_DIR/qpid-python-test -m dtx -m persistence -b localhost:$env:QPID_PORT $fails $tests
$RETCODE=$LASTEXITCODE
if ($RETCODE -ne 0) {
   $FAILCODE = 1
}
# Piping the output makes the script wait for qpidd to finish.
Invoke-Expression "$prog --quit --port $env:QPID_PORT" | Write-Output


# Test 2... store.py starts/stops/restarts its own brokers

# This only works with the brokertest.py changes made by Steve - see QPID-2492.
#$tests = "*"
#$env:PYTHONPATH="$PYTHON_DIR;$srcdir"
#$env:QPIDD_EXEC="$prog"
#$env:STORE_LIB="$store_dir\store$suffix.dll"
#$env:STORE_SQL_LIB="$store_dir\mssql_store$suffix.dll"
#$env:STORE_CATALOG="store_recovery_test_$stamp"
#Invoke-Expression "python $PYTHON_DIR/qpid-python-test -m store $tests" | Out-Default
#$RETCODE=$LASTEXITCODE
#if ($RETCODE -ne 0) {
#    "FAIL store tests"
#    $FAILCODE = 1
#}
exit $FAILCODE