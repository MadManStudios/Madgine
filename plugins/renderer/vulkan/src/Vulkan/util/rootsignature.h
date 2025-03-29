RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS),
CBV(b0),
CBV(b1),
CBV(b2),
CBV(b3),
DescriptorTable(SRV(t0)),
DescriptorTable(SRV(t1)),
DescriptorTable(SRV(t2)),
DescriptorTable(SRV(t3)),
DescriptorTable(SRV(t4)),

StaticSampler(s0,
				filter = FILTER_MIN_MAG_MIP_LINEAR,
				addressU = TEXTURE_ADDRESS_WRAP,
				addressV = TEXTURE_ADDRESS_WRAP,
				addressW = TEXTURE_ADDRESS_WRAP
				),

StaticSampler(s1,
				filter = FILTER_MIN_MAG_MIP_LINEAR,
				addressU = TEXTURE_ADDRESS_CLAMP,
				addressV = TEXTURE_ADDRESS_CLAMP,
				addressW = TEXTURE_ADDRESS_CLAMP
				)