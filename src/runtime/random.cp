from ltypes import i32, f64, ccall

e: f64 = 2.718281828459045235360287471352662497757
eps: f64 = 1e-16

def random() -> f64:
    """
    Returns a random floating point number in the range [0.0, 1.0)
    """
    return _lfortran_random()

@ccall
def _lfortran_random() -> f64:
    pass

def randrange(lower: i32, upper: i32) -> i32:
    """
    Return a random integer N such that `lower <= N < upper`.
    """
    return _lfortran_randrange(lower, upper)

@ccall
def _lfortran_randrange(lower: i32, upper: i32) -> i32:
    pass

def randint(lower: i32, upper: i32) -> i32:
    """
    Return a random integer N such that `lower <= N <= upper`.
    """
    return _lfortran_random_int(lower, upper)

@ccall
def _lfortran_random_int(lower: i32, upper: i32) -> i32:
    pass
