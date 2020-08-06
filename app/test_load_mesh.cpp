#include "load_mesh.hpp"
#include <cassert>

using namespace demo::mesh;

void test_off_parse_counts()
{
    std::size_t vertex_count;
    std::size_t face_count;

    std::string test("5 10 0");
    assert(off_parse_counts(test, vertex_count, face_count));
    assert(vertex_count == 5);
    assert(face_count == 10);


    test = "  \t   4 6\t0";
    assert(off_parse_counts(test, vertex_count, face_count));
    assert(vertex_count == 4);
    assert(face_count == 6);

    test = " \t\t           ";
    assert(!off_parse_counts(test, vertex_count, face_count));

    test = "#this is a comment";
    assert(!off_parse_counts(test, vertex_count, face_count));

    test = "\t  #this is a comment";
    assert(!off_parse_counts(test, vertex_count, face_count));

    test = "this is invalid";
    assert(off_parse_counts(test, vertex_count, face_count));
    assert(vertex_count == 0 || face_count == 0);
}

int main()
{
    test_off_parse_counts();

    return 0;
}
