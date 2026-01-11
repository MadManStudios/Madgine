
ByteAddressBuffer memory[4] : register(t0, space0);

template <typename T>
struct ArrayPtr{
    T Load()
    {
		return memory[buffer() - 1].template Load<T>(offset());
    }

    uint offset()
    {
        return data.x;
    }

    uint buffer()
    {
        return data.y;
    }
	
	uint2 data;
};

template <>
struct ArrayPtr<float4x4>{
	float4x4 operator[](uint index){
		return memory[buffer() - 1].Load<float4x4>(offset() + 64 * index);
	}

	operator bool(){
		return buffer() != 0;
	}

	uint offset() {
		return data.x;
	}

	uint buffer() {
		return data.y;
	}

	uint2 data;
};
