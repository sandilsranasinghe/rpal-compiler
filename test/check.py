import argparse

parser = argparse.ArgumentParser(description='Check test results')
parser.add_argument('test_name', type=str, help='Name of test to check')
# flag for verbose
parser.add_argument('--verbose', dest='verbose', action='store_true')

if __name__ == "__main__":
    args = parser.parse_args()
    test_name = args.test_name
    verbose = args.verbose
    expected = ""
    result = ""

    with open("{}.txt".format(test_name), "r") as f:
        expected = f.read()

    with open("{}.out.txt".format(test_name), "r") as f:
        result = f.read()

    if expected == result:
        print("Test {} passed".format(test_name))
    else:
        print("Test {} failed".format(test_name))
        if verbose:
            print("Expected: {}".format(expected))
            print("Result: {}".format(result))