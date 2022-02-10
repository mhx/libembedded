#!/usr/bin/python

import argparse
from elftools.elf.elffile import ELFFile
from struct import unpack


def parse(data):
    if data.startswith(b"TLIF"):
        raise RuntimeError("big endian byte order not implemented")
    elif not data.startswith(b"FILT"):
        raise RuntimeError("invalid section data")

    valtypes = {
        0: "f",
        1: "d",
    }

    valsizes = {
        0: 4,
        1: 8,
    }

    soscoef = ["b0", "b1", "b2", "a1", "a2"]

    while len(data) > 0:
        header_size = 128
        magic, size, version, structure, valtype, name = unpack(
            "IHBBB119s", data[:header_size]
        )
        assert magic == 0x544C4946
        assert version == 0
        size -= header_size
        data = data[header_size:]
        valnum = size // valsizes[valtype]
        values = unpack(f"{valnum}{valtypes[valtype]}", data[:size])
        data = data[size:]
        name = name.rstrip(b"\0").decode("utf-8")
        print(f"{name}:")

        if structure == 0:
            sosnum = valnum // 5
            for i in range(sosnum):
                print(f"  SOS stage {i + 1}:")
                for k in range(5):
                    print(f"    {soscoef[k]} = {values[5*i + k]}")
        elif structure == 1:
            polynum = valnum // 2
            for k, coef in enumerate(["b", "a"]):
                for i in range(polynum):
                    print(f"  {coef}[{i}] = {values[polynum*k + i]}")
        else:
            raise RuntimeError("unsupported filter structure: {structure}")

        npos = data.find(b"FILT")
        if npos > 0:
            data = data[npos:]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Filter coefficient parser")
    parser.add_argument("object", type=str, help="object file")
    opt = parser.parse_args()

    with open(opt.object, "rb") as fh:
        elf = ELFFile(fh)
        coef = elf.get_section_by_name(".libemb_filter_coefs")
        parse(coef.data())
