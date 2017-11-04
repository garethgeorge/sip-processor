import os 
import subprocess
import random 

BIN = "../schmib_q/bmbp_ts"

fulldataset = []
for i in range(0, 10):
    offset = random.gauss(10000, 1000)
    dataset = [("%d %.4f\n" % (x, offset + random.gauss(1000, 300))).encode('ascii') for x in range(len(fulldataset), len(fulldataset) + 800)]
    fulldataset.extend(dataset)

def splitdataset(dataset, parts=2):
    split_points = random.sample(range(1, len(fulldataset)), parts - 1)
    split_points.sort()

    fnames = []
    last_point = 0
    for point in split_points:
        samples = fulldataset[last_point:point]
        fname = "input-%d.txt" % point
        with open(fname, 'wb') as f:
            f.writelines(samples)
        fnames.append(fname)
        last_point = point 
    
    with open("therest.txt", "wb") as f:
        f.writelines(fulldataset[last_point:])
    fnames.append("therest.txt")
    
    return fnames 

def run_bmbp_ts(fname, savestate=None, loadstate=None):
    cmd = BIN + " -T --file " + fname + " --quantile 0.97 --confidence 0.01"
    if savestate != None:
        cmd += " --savestate " + savestate 
    if loadstate != None:
        cmd += " --loadstate " + loadstate
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
    return p

def cleanup_files(dir, extension='.txt'):
    filelist = [f for f in os.listdir(dir) if f.endswith(extension)]
    for f in filelist:
        os.remove(f)


expected_success_percent = None 

def test_can_run_bmbp_ts_on_dataset():
    global expected_success_percent

    with open("input.txt", "wb") as input:
        input.writelines(fulldataset)
    p = run_bmbp_ts("input.txt", savestate=None, loadstate=None)
    output = p.stdout.read().decode('ascii').split("\n")
    p.wait()
    assert(p.returncode == 0)
    expected_success_percent = output[-2]

    print("Success percentage for full dataset:")
    print(output[-2])

def test_can_get_same_success_with_one_split():
    with open("input1.txt", "wb") as input:
        input.writelines(fulldataset[0:round(len(fulldataset) * 0.5)])
    
    with open("input2.txt", "wb") as input:
        input.writelines(fulldataset[round(len(fulldataset) * 0.5):])

    p = run_bmbp_ts("input1.txt", savestate="state.txt", loadstate=None)
    p.stdout.read()
    p.wait()
    assert(p.returncode == 0)

    p = run_bmbp_ts("input2.txt", savestate=None, loadstate="state.txt")
    output = p.stdout.read().decode('ascii').split("\n")
    p.wait()
    assert(p.returncode == 0)

    assert(output[-2] == expected_success_percent)
    print("Success percentage with split at half way: ")
    print(output[-2])

def test_can_get_same_success_with_random_splits():
    files = splitdataset(fulldataset, parts=10)

    last_state = None
    last_succ_percentage = None
    for f in files:
        print("\trunning fragment: %s" % f)
        
        p = run_bmbp_ts(f, savestate="state.txt", loadstate=last_state)
        output = p.stdout.read().decode('ascii').split("\n")
        p.wait()
        assert(p.returncode == 0)

        last_state = "state.txt"

        print("\tfragment success percentage:")
        print("\t\t" + output[-2])
    
        last_succ_percentage = output[-2]

    assert(last_succ_percentage == expected_success_percent)
