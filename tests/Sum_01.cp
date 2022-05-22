from ltypes import i32

def is_prime(n: i32) -> bool:
    factors: i32
    factors = 0
    i: i32
    for i in range(2, n):
        if n % i == 0:
            factors += 1
    return factors == 0

def main():
    limit: i32
    limit = 20000
    total: i32
    total = 0
    i: i32
    for i in range(2, limit):
        if is_prime(i):
            total += 1
    print(total)

main()
