// <auto-generated>
//  automatically generated by the FlatBuffers compiler, do not modify
// </auto-generated>

namespace MyGame.Example
{

using global::System;
using global::System.Collections.Generic;
using global::FlatBuffers;

public struct Test : IFlatbufferObject
{
  private Struct __p;
  public ByteBuffer ByteBuffer { get { return __p.bb; } }
  public void __init(int _i, ByteBuffer _bb) { __p = new Struct(_i, _bb); }
  public Test __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public short A { get { return __p.bb.GetShort(__p.bb_pos + 0); } }
  public void MutateA(short a) { __p.bb.PutShort(__p.bb_pos + 0, a); }
  public sbyte B { get { return __p.bb.GetSbyte(__p.bb_pos + 2); } }
  public void MutateB(sbyte b) { __p.bb.PutSbyte(__p.bb_pos + 2, b); }

  public static Offset<MyGame.Example.Test> CreateTest(FlatBufferBuilder builder, short A, sbyte B) {
    builder.Prep(2, 4);
    builder.Pad(1);
    builder.PutSbyte(B);
    builder.PutShort(A);
    return new Offset<MyGame.Example.Test>(builder.Offset);
  }
  public TestT UnPack() {
    var _o = new TestT();
    this.UnPackTo(_o);
    return _o;
  }
  public void UnPackTo(TestT _o) {
    _o.A = this.A;
    _o.B = this.B;
  }
  public static Offset<MyGame.Example.Test> Pack(FlatBufferBuilder builder, TestT _o) {
    if (_o == null) return default(Offset<MyGame.Example.Test>);
    return CreateTest(
      builder,
      _o.A,
      _o.B);
  }
}

public class TestT
{
  [Newtonsoft.Json.JsonProperty("a")]
  public short A { get; set; }
  [Newtonsoft.Json.JsonProperty("b")]
  public sbyte B { get; set; }

  public TestT() {
    this.A = 0;
    this.B = 0;
  }
}


}