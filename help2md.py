import re
import sys

FOOTNOTES = {
    "1": "Also for complex operands.",
    "2": "Complex conjugate: ~(1+2i)=1-2i",
    "3": "Also for matrix operands.",
    "4": "Matrix inversion."
}


def convert_superscripts(line):
    """
    Convert (1), (1,3) → <sup>[1](#fn1),[3](#fn3)</sup>
    """

    def repl(match):
        nums = match.group(1)
        parts = [n.strip() for n in nums.split(",")]

        links = []
        for p in parts:
            if p in FOOTNOTES:
                links.append(f"[{p}](#fn{p})")
            else:
                links.append(p)

        return "<sup>" + ",".join(links) + "</sup>"

    return re.sub(r"\(([\d,\s]+)\)", repl, line)


def process_file(text, mode="strict"):
    lines = text.splitlines()

    out = []

    if mode == "strict":
        out.append("```")

    for line in lines:
        new_line = convert_superscripts(line)
        out.append(new_line)

    if mode == "strict":
        out.append("```")

    # footnotes
    out.append("\n---\n")
    out.append("## Footnotes\n")

    for k, v in FOOTNOTES.items():
        out.append(f'<a id="fn{k}"></a>({k}) {v}  ')

    return "\n".join(out)


def main():
    if len(sys.argv) < 3:
        print("Usage: help2md.py input.txt output.md [strict|pretty]")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    mode = sys.argv[3] if len(sys.argv) > 3 else "strict"

    with open(input_file, "r", encoding="utf-8") as f:
        text = f.read()

    result = process_file(text, mode)

    with open(output_file, "w", encoding="utf-8") as f:
        f.write(result)

    print(f"Done: {output_file}")


if __name__ == "__main__":
    main()