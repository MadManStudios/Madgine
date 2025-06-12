
struct Test
{
    float4 value;
};

export float4 Foo0()
{
    return 1.0f;
}

export float4 Foo1(float4 input1)
{
    return input1 + 1.0f;
}

export float4 Foo2(float4 input1, float4 input2)
{
    return input1 + 1.0f;
}


export void Bar()
{
}

export void Bar1(float)
{
}

export float4 gammaCorrection(float4 input, float4 input2, float4 input4, Test input3)
{		
    return Foo1(input + input2) + input3.value;
}
