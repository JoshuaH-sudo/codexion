import subprocess
import sys

candidates = [950, 1050, 1150, 1250, 1350, 1450, 1550, 1650, 1750, 1850, 1950, 2050]
args_template = "5 {} 200 200 200 3 50 edf"

def run_trials(limit, trials):
    burnouts = 0
    for _ in range(trials):
        cmd = f"./codexion {args_template.format(limit)}"
        try:
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
            if "burned out" in result.stdout or "burned out" in result.stderr:
                burnouts += 1
        except Exception:
            burnouts += 1
    return burnouts

print("| Limit | Burnouts (x/5) |")
print("|-------|----------------|")
first_zero = None
for c in candidates:
    b = run_trials(c, 5)
    print(f"| {c:5} | {b}/5          |")
    if b == 0 and first_zero is None:
        first_zero = c

if first_zero:
    print(f"\nFirst all-pass (0/5) found at: {first_zero}")
    print("\nNarrowing around", first_zero)
    print("| Limit | Burnouts (x/10)|")
    print("|-------|----------------|")
    # Step 25 around first_zero +/- 100
    narrow_candidates = sorted(list(set(range(max(750, first_zero - 100), first_zero + 101, 25))))
    first_zero_10 = None
    for c in narrow_candidates:
        b = run_trials(c, 10)
        print(f"| {c:5} | {b}/10         |")
        if b == 0 and first_zero_10 is None:
            first_zero_10 = c
    
    if first_zero_10:
        print(f"\nResults:")
        print(f"1) First burnout value with 0/5 failures: {first_zero}")
        print(f"2) First burnout value with 0/10 failures: {first_zero_10}")
        print(f"3) Suggested practical margin: {first_zero_10 - 750}")
