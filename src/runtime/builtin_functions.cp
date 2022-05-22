from ltypes import i8, i16, i32, i64, f32, f64, c32, c64, overload
#from sys import exit

"""
#: abs() as a generic procedure.
#: supported types for argument:
#: i8, i16, i32, i64, f32, f64, bool, c32, c64
"""
@overload
def abs(x: f64) -> f64:
    """
    Return the absolute value of `x`.
    """
    if x >= 0.0:
        return x
    else:
        return -x

@overload
def abs(x: f32) -> f32:
    if x >= 0.0:
        return x
    else:
        return -x

@overload
def abs(x: i8) -> i8:
    if x >= 0:
        return x
    else:
        return -x

@overload
def abs(x: i16) -> i16:
    if x >= 0:
        return x
    else:
        return -x

@overload
def abs(x: i32) -> i32:
    if x >= 0:
        return x
    else:
        return -x

@overload
def abs(x: i64) -> i64:
    if x >= 0:
        return x
    else:
        return -x

@overload
def abs(b: bool) -> i32:
    if b:
        return 1
    else:
        return 0

@overload
def abs(c: c32) -> f32:
    a: f32
    b: f32
    a = c.real
    b = _lfortran_caimag(c)
    return (a**2 + b**2)**(1/2)

@overload
def abs(c: c64) -> f64:
    a: f64
    b: f64
    a = c.real
    b = _lfortran_zaimag(c)
    return (a**2 + b**2)**(1/2)


def str(x: i32) -> str:
    """
    Return the string representation of an integer `x`.
    """
    if x == 0:
        return '0'
    result: str
    result = ''
    if x < 0:
        result += '-'
        x = -x
    rev_result: str
    rev_result = ''
    rev_result_len: i32
    rev_result_len = 0
    pos_to_str: list[str]
    pos_to_str = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
    while x > 0:
        rev_result += pos_to_str[x - _compy_floordiv(x, 10)*10]
        rev_result_len += 1
        x = _compy_floordiv(x, 10)
    pos: i32
    for pos in range(rev_result_len - 1, -1, -1):
        result += rev_result[pos]
    return result

"""
#: bool() as a generic procedure.
#: supported types for argument:
#: i8, i16, i32, i64, f32, f64, bool
"""
@overload
def bool(x: i32) -> bool:
    """
    Return False when the argument `x` is 0, True otherwise.
    """
    return x != 0

@overload
def bool(x: i64) -> bool:
    return x != 0

@overload
def bool(x: i8) -> bool:
    return x != 0

@overload
def bool(x: i16) -> bool:
    return x != 0

@overload
def bool(f: f32) -> bool:
    return f != 0.0

@overload
def bool(f: f64) -> bool:
    """
    Return False when the argument `x` is 0.0, True otherwise.
    """
    return f != 0.0

@overload
def bool(s: str) -> bool:
    """
    Return False when the argument `s` is an empty string, True otherwise.
    """
    return len(s) > 0

@overload
def bool(b: bool) -> bool:
    return b

@overload
def bool(c: c32) -> bool:
    return c.real != 0.0 or _lfortran_caimag(c) != 0.0

@overload
def bool(c: c64) -> bool:
    return c.real != 0.0 or _lfortran_zaimag(c) != 0.0

@interface
def len(s: str) -> i32:
    """
    Return the length of the string `s`.
    """
    pass

"""
#: pow() as a generic procedure.
#: supported types for arguments:
#: (i32, i32), (i64, i64), (f64, f64),
#: (f32, f32), (i32, f64), (f64, i32),
#: (i32, f32), (f32, i32), (bool, bool), (c32, i32)
"""
@overload
def pow(x: i32, y: i32) -> i32:
    """
    Returns x**y.
    """
    return x**y

@overload
def pow(x: i64, y: i64) -> i64:
    return x**y

@overload
def pow(x: f32, y: f32) -> f32:
    return x**y

@overload
def pow(x: f64, y: f64) -> f64:
    """
    Returns x**y.
    """
    return x**y

@overload
def pow(x: i32, y: f32) -> f32:
    return x**y

@overload
def pow(x: f32, y: i32) -> f32:
    return x**y

@overload
def pow(x: i32, y: f64) -> f64:
    return x**y

@overload
def pow(x: f64, y: i32) -> f64:
    return x**y

@overload
def pow(x: bool, y: bool) -> i32:
    if y and not x:
        return 0

    return 1

@overload
def pow(c: c32, y: i32) -> c32:
    return c**y

"""
#: round() as a generic procedure.
#: supported types for argument:
#: i8, i16, i32, i64, f32, f64, bool
"""
@overload
def round(value: f64) -> i32:
    """
    Rounds a floating point number to the nearest integer.
    """
    i: i32
    i = int(value)
    f: f64
    f = abs(value - i)
    if f < 0.5:
        return i
    elif f > 0.5:
        return i + 1
    else:
        if i - _compy_floordiv(i, 2) * 2 == 0:
            return i
        else:
            return i + 1

@overload
def round(value: f32) -> i32:
    i: i32
    i = int(value)
    f: f64
    f = abs(value - i)
    if f < 0.5:
        return i
    elif f > 0.5:
        return i + 1
    else:
        if i - _compy_floordiv(i, 2) * 2 == 0:
            return i
        else:
            return i + 1

@overload
def round(value: i32) -> i32:
    return value

@overload
def round(value: i64) -> i64:
    return value

@overload
def round(value: i8) -> i8:
    return value

@overload
def round(value: i16) -> i16:
    return value

@overload
def round(b: bool) -> i32:
    return abs(b)

@interface
def divmod(x: i32, y: i32) -> tuple[i32, i32]:
    #: TODO: Implement once we have tuple support in the LLVM backend
    pass


def lbound(x: i32[:], dim: i32) -> i32:
    pass


def ubound(x: i32[:], dim: i32) -> i32:
    pass


@ccall
def _lfortran_caimag(x: c32) -> f32:
    pass

@ccall
def _lfortran_zaimag(x: c64) -> f64:
    pass

@overload
def _compy_imag(x: c64) -> f64:
    return _lfortran_zaimag(x)

@overload
def _compy_imag(x: c32) -> f32:
    return _lfortran_caimag(x)


@overload
def _compy_floordiv(a: f64, b: f64) -> f64:
    r: f64
    r = a/b
    result: i64
    result = int(r)
    if r >= 0.0 or result == r:
        return float(result)
    return float(result-1)


@overload
def _compy_floordiv(a: f32, b: f32) -> f32:
    r: f32
    r = a/b
    result: i32
    result = int(r)
    if r >= 0.0 or result == r:
        return float(result)
    return float(result-1)

@overload
def _compy_floordiv(a: i32, b: i32) -> i32:
    r: f32
    r = a/b
    result: i32
    result = int(r)
    if r >= 0.0 or result == r:
        return result
    return result - 1

@overload
def _compy_floordiv(a: i64, b: i64) -> i64:
    r: f64
    r = a/b
    result: i64
    result = int(r)
    if r >= 0.0 or result == r:
        return result
    return result - 1


@overload
def _mod(a: i32, b: i32) -> i32:
    return a - _compy_floordiv(a, b)*b

@overload
def _mod(a: f32, b: f32) -> f32:
    return a - _compy_floordiv(a, b)*b

@overload
def _mod(a: i64, b: i64) -> i64:
    return a - _compy_floordiv(a, b)*b

@overload
def _mod(a: f64, b: f64) -> f64:
    return a - _compy_floordiv(a, b)*b


@overload
def max(a: i32, b: i32) -> i32:
    if a > b:
        return a
    else:
        return b

@overload
def max(a: i32, b: i32, c: i32) -> i32:
    res: i32 = a
    if b > res:
        res = b
    if c > res:
        res = c
    return res

@overload
def max(a: f64, b: f64, c: f64) -> f64:
    res: f64 =a
    if b - res > 1e-6:
        res = b
    if c - res > 1e-6:
        res = c
    return res

@overload
def max(a: f64, b: f64) -> f64:
    if a - b > 1e-6:
        return a
    else:
        return b

@overload
def min(a: i32, b: i32) -> i32:
    if a < b:
        return a
    else:
        return b

@overload
def min(a: i32, b: i32, c: i32) -> i32:
    res: i32 = a
    if b < res:
        res = b
    if c < res:
        res = c
    return res

@overload
def min(a: f64, b: f64, c: f64) -> f64:
    res: f64 = a
    if res - b > 1e-6:
        res = b
    if res - c > 1e-6:
        res = c
    return res

@overload
def min(a: f64, b: f64) -> f64:
    if b - a > 1e-6:
        return a
    else:
        return b

@overload
def _bitwise_or(a: i32, b: i32) -> i32:
    pass

@overload
def _bitwise_or(a: i64, b: i64) -> i64:
    pass

@overload
def _bitwise_and(a: i32, b: i32) -> i32:
    pass

@overload
def _bitwise_and(a: i64, b: i64) -> i64:
    pass

@overload
def _bitwise_xor(a: i32, b: i32) -> i32:
    pass

@overload
def _bitwise_xor(a: i64, b: i64) -> i64:
    pass

@overload
def _bitwise_lshift(a: i32, b: i32) -> i32:
    if b < 0:
        raise ValueError("Negative shift count not allowed.")
    return a*2**b

@overload
def _bitwise_lshift(a: i64, b: i64) -> i64:
    if b < 0:
        raise ValueError("Negative shift count not allowed.")
    return a*2**b

@overload
def _bitwise_rshift(a: i32, b: i32) -> i32:
    if b < 0:
        raise ValueError("Negative shift count not allowed.")
    i: i32
    i = 2
    return _compy_floordiv(a, i**b)

@overload
def _bitwise_rshift(a: i64, b: i64) -> i64:
    if b < 0:
        raise ValueError("Negative shift count not allowed.")
    i: i64
    i = 2
    return _compy_floordiv(a, i**b)
