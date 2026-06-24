#!/usr/bin/env python3
"""Make AirSLAM's int32-typecast ONNX models parse under TensorRT 10.

The shipped *_sim_int32.onnx models were produced by `onnx-typecast` (int64->int32)
to satisfy TensorRT 8, which lacked native int64 support. TensorRT 10 supports int64
but is strict about type consistency: a Concat (and a few other ops) whose inputs mix
Int32 (typecast constants) and Int64 (e.g. Shape outputs) is rejected:

    Concat_43: concat input tensors 0 and 3 have incompatible types Int64 and Int32

This pass runs ONNX shape/type inference and, for every op whose integer inputs are a
mix of INT32 and INT64, inserts Cast(to=INT64) on the INT32 inputs so all inputs agree
(int64 is the natural type for the shape arithmetic these feed). Idempotent.

Usage: fix_onnx_int_types_for_trt10.py <in.onnx> [out.onnx]   (defaults to in-place)
"""
import sys
import onnx
from onnx import TensorProto, helper

# Ops where TRT 10 requires all (integer) inputs to share a type: variadic
# concat/elementwise reducers, binary elementwise math, and comparisons. The
# int32-typecast left int32 constants feeding shape arithmetic that elsewhere uses
# int64 (Shape outputs), so these are exactly the nodes TRT 10 rejects.
TYPE_SENSITIVE_OPS = {
    "Concat", "Where", "ScatterND", "ScatterElements",
    "Add", "Sub", "Mul", "Div", "Pow", "Mod",
    "Min", "Max", "Mean", "Sum",
    "And", "Or", "Xor", "BitShift",
    "Greater", "GreaterOrEqual", "Less", "LessOrEqual", "Equal",
}


def elem_types(model):
    """name -> elem_type, from inputs, initializers, and inferred value_info."""
    m = onnx.shape_inference.infer_shapes(model)
    t = {}
    for vi in list(m.graph.value_info) + list(m.graph.input) + list(m.graph.output):
        if vi.type.HasField("tensor_type"):
            t[vi.name] = vi.type.tensor_type.elem_type
    for init in m.graph.initializer:
        t[init.name] = init.data_type
    return t


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    in_path = sys.argv[1]
    out_path = sys.argv[2] if len(sys.argv) > 2 else in_path

    model = onnx.load(in_path)
    types = elem_types(model)
    g = model.graph

    n_cast = 0
    new_nodes = []
    for node in g.node:
        if node.op_type in TYPE_SENSITIVE_OPS:
            in_types = [(i, types.get(inp)) for i, inp in enumerate(node.input) if inp]
            present = {ty for _, ty in in_types if ty in (TensorProto.INT32, TensorProto.INT64)}
            if present == {TensorProto.INT32, TensorProto.INT64}:
                for idx, ty in in_types:
                    if ty == TensorProto.INT32:
                        src = node.input[idx]
                        dst = f"{src}_to_i64_{node.name}_{idx}"
                        new_nodes.append(helper.make_node(
                            "Cast", [src], [dst], to=TensorProto.INT64,
                            name=f"Cast_{node.name}_{idx}"))
                        node.input[idx] = dst
                        n_cast += 1
        new_nodes.append(node)

    if n_cast == 0:
        print(f"{in_path}: no mixed int32/int64 ops found; unchanged")
        return

    del g.node[:]
    g.node.extend(new_nodes)
    onnx.checker.check_model(model)
    onnx.save(model, out_path)
    print(f"{in_path}: inserted {n_cast} int32->int64 Cast(s) -> {out_path}")


if __name__ == "__main__":
    main()
