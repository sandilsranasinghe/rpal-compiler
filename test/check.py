import argparse

parser = argparse.ArgumentParser(description="Check test results")
parser.add_argument("test_name", type=str, help="Name of test to check")
# flag for verbose
parser.add_argument("--verbose", dest="verbose", action="store_true")


class OutputColor:
    BLUE = "\033[94m"
    CYAN = "\033[96m"
    GREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"

    @classmethod
    def print_success(cls, msg):
        print(cls.GREEN + msg + cls.ENDC)

    @classmethod
    def print_warning(cls, msg):
        print(cls.WARNING + msg + cls.ENDC)

    @classmethod
    def print_fail(cls, msg):
        print(cls.FAIL + msg + cls.ENDC)

output_color = OutputColor()

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
        output_color.print_success("Test {} passed".format(test_name))
    else:
        output_color.print_fail("Test {} failed".format(test_name))
        if verbose:
            output_color.print_warning("Expected: {}".format(expected))
            output_color.print_warning("Result: {}".format(result))
