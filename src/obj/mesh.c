#include <obj/mesh.h>
int mesh_count_faces(Mesh *self)
{
    int base_len = self->vertices.end - self->vertices.start;

    return base_len / (MESH_VERTICE_COUNT);
}

Triangle triangle_unpack(const Vec3 *packed_data)
{
    Triangle triangle;

    // position
    triangle.pa = packed_data[0];
    triangle.pb = packed_data[1];
    triangle.pc = packed_data[2];

    // texcoords
    Vec3 tcoord1 = packed_data[3];
    Vec3 tcoord2 = packed_data[4];

    triangle.tex_coords[0][0] = tcoord1.x; // pa.x
    triangle.tex_coords[0][1] = tcoord1.y; // pa.y
    triangle.tex_coords[1][0] = tcoord1.z; // pb.x
    triangle.tex_coords[1][1] = tcoord2.x; // pb.y
    triangle.tex_coords[2][0] = tcoord2.y; // pc.x
    triangle.tex_coords[2][1] = tcoord2.z; // pc.y

    // normals

    triangle.na = packed_data[5];
    triangle.nb = packed_data[6];
    triangle.nc = packed_data[7];

    // Tangent

    triangle.ta = packed_data[8];
    triangle.tb = packed_data[9];
    triangle.tc = packed_data[10];
    return triangle;
}

void triangle_pack(Vec3 *target, Triangle triangle)
{

    // position
    target[0] = triangle.pa;
    target[1] = triangle.pb;
    target[2] = triangle.pc;

    // texcoords
    Vec3 tcoord1;
    Vec3 tcoord2;

    tcoord1.x = triangle.tex_coords[0][0]; // pa.x
    tcoord1.y = triangle.tex_coords[0][1]; // pa.y
    tcoord1.z = triangle.tex_coords[1][0]; // pb.x
    tcoord2.x = triangle.tex_coords[1][1]; // pb.y
    tcoord2.y = triangle.tex_coords[2][0]; // pc.x
    tcoord2.z = triangle.tex_coords[2][1]; // pc.y

    target[3] = tcoord1;
    target[4] = tcoord2;

    // normals

    target[5] = triangle.na;
    target[6] = triangle.nb;
    target[7] = triangle.nc;

    // Tangent

    target[8] = triangle.ta;
    target[9] = triangle.tb;
    target[10] = triangle.tc;
}
