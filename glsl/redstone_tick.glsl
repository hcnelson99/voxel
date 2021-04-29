#version 430

layout(packed, binding = 0) buffer buffer0 { uint block_map[]; };
layout(packed, binding = 2) buffer buffer1 { uint expression_values[]; };
layout(packed, binding = 4) buffer buffer2 { uint block_to_expression[]; };

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

uint world_size = gl_WorkGroupSize.x * gl_NumWorkGroups.x;

const uint[] inactive_map = uint[](
    /*Block::Air*/ 0,
    /*Block::Stone*/ 1,
    /*Block::Dirt*/ 2,
    /*Block::Wood*/ 3,
    /*Block::InactiveRedstone*/ 5,
    /*Block::InactiveRedstone*/ 5,
    /*Block::DelayGate*/ 7,
    /*Block::DelayGate*/ 7,
    /*Block::NotGate*/ 9,
    /*Block::NotGate*/ 9,
    /*Block::DiodeGate*/ 11,
    /*Block::DiodeGate*/ 11,
    /*Block::Display*/ 13,
    /*Block::Display*/ 13,
    /*Block::Switch*/ 15,
    /*Block::Switch*/ 15,
    /*Block::InactiveBluestone*/ 17,
    /*Block::InactiveBluestone*/ 17,
    /*Block::InactiveGreenstone*/ 19,
    /*Block::InactiveGreenstone*/ 19
);

const uint[] active_map = uint[](
    /*Block::Air*/ 0,
    /*Block::Stone*/ 1,
    /*Block::Dirt*/ 2,
    /*Block::Wood*/ 3,
    /*Block::ActiveRedstone*/ 4,
    /*Block::ActiveRedstone*/ 4,
    /*Block::ActiveDelayGate*/ 6,
    /*Block::ActiveDelayGate*/ 6,
    /*Block::ActiveNotGate*/ 8,
    /*Block::ActiveNotGate*/ 8,
    /*Block::ActiveDiodeGate*/ 10,
    /*Block::ActiveDiodeGate*/ 10,
    /*Block::ActiveDisplay*/ 12,
    /*Block::ActiveDisplay*/ 12,
    /*Block::ActiveSwitch*/ 14,
    /*Block::ActiveSwitch*/ 14,
    /*Block::ActiveBluestone*/ 16,
    /*Block::ActiveBluestone*/ 16,
    /*Block::ActiveGreenstone*/ 18,
    /*Block::ActiveGreenstone*/ 18
);

uint get_block(ivec3 v) {
    uint _index = v.x * world_size * world_size + v.y * world_size + v.z;
    uint element_index = _index / 4;
    uint byte_index = 8 * (_index % 4);

    return (block_map[element_index] >> byte_index) & 0xFF;
}

void set_block(ivec3 v, uint block) {
    uint _index = v.x * world_size * world_size + v.y * world_size + v.z;
    uint element_index = _index / 4;
    uint byte_index = 8 * (_index % 4);

    block_map[element_index] &= ~(0xFF << byte_index);
    block_map[element_index] |= block << byte_index;
}

bool expression_active(uint i) {
    return ((expression_values[i / 4] >> (8 * (i % 4))) & 0xFF) != 0;
}

void set_active(ivec3 v, bool block_active) {
    uint block = get_block(v);
    uint new_block;
    if (block_active) {
        new_block = (active_map[block >> 3] << 3) | (block & 7);
    } else {
        new_block = (inactive_map[block >> 3] << 3) | (block & 7);
    }
    set_block(v, new_block);
}

void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz) * ivec3(1, 1, 4);

    for (int i = 0; i < 4; i++) {
        uint index = pos.x * world_size * world_size + pos.y * world_size + pos.z;
        uint expr_i = block_to_expression[index];
        bool block_active = expression_active(expr_i);
        set_active(pos, block_active);

        pos = pos + ivec3(0, 0, 1);
    }
}
