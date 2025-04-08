import subprocess
import collections
import json
from tqdm import tqdm
import math

QTEST_PATH = './qtest'
ELEMENTS = ['3', '2', '1']
REPEAT = 1000000

def compute_entropy(counter):
    total = sum(counter.values())
    entropy = 0.0
    for count in counter.values():
        p = count / total
        entropy -= p * math.log2(p)
    return entropy

def main():
    counter = collections.Counter()

    process = subprocess.Popen(
        [QTEST_PATH, "-v", "3"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        bufsize=1
    )

    def send(cmd):
        process.stdin.write(cmd + '\n')
        process.stdin.flush()

    def readline():
        return process.stdout.readline().strip()

    send('new')
    readline()
    for e in ELEMENTS:
        send(f'ih {e}')
        readline()

    for _ in tqdm(range(REPEAT), desc="Shuffling", unit="iter"):
        result = None
        while True:
            send('shuffle')
            line = readline()
            if line.startswith("l = ["):
                content = line[line.find('[')+1:line.find(']')]
                result = "".join(content.split())
                break
        if result:
            counter[result] += 1

        send('sort')
        while True:
            line = readline()
            if line.startswith("l = ["):
                break

    send('quit')
    process.stdin.close()
    process.wait()

    print("\n=== Shuffle Distribution ===")
    total = sum(counter.values())
    for perm in sorted(counter.keys()):
        freq = counter[perm]
        print(f"{perm}: {freq} ({freq/total:.4%})")

    with open('shuffle_results.json', 'w') as f:
        json.dump(counter, f, indent=2)

    entropy = compute_entropy(counter)
    max_entropy = math.log2(len(counter))
    print("\n=== Shannon Entropy ===")
    print(f"Entropy: {entropy:.4f} bits")
    print(f"Max entropy: {max_entropy:.4f} bits")
    print(f"Ratio: {entropy / max_entropy:.2%}")

if __name__ == '__main__':
    main()
