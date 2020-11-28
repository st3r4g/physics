#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

/*int readFile(const char *path, size_t *nbyte_, void **buf_) {
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	size_t nbyte = lseek(fd, 0, SEEK_END);
	if (nbyte < 0)
		goto clean0;
	void *buf = malloc(nbyte);
	if (!buf)
		goto clean0;
	if (lseek(fd, 0, SEEK_SET) < 0)
		goto clean1;
	if (read(fd, buf, nbyte) < 0)
		goto clean1;
	if (close(fd) < 0)
		goto clean1;

	*nbyte_ = nbyte;
	*buf_ = buf;
	return 0;
clean1:
	free(buf);
clean0:
	close(fd);
	return -1;
}*/

void file_reader(const char *filename, int is_mtl, const char *obj_filename,
                 char **buf, size_t *len) {
    readFile(filename, len, (void**)buf);
}

void loadModel(size_t *num_vertices, float **vertices_) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes;
    tinyobj_material_t *materials;
    size_t num_shapes, num_materials;
    tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials,
                      "cube.obj", file_reader, TINYOBJ_FLAG_TRIANGULATE);
    float *vertices = malloc(3*attrib.num_faces*sizeof(float));
    for (int i=0; i<attrib.num_faces; i++) {
        vertices[3*i] = attrib.vertices[3*attrib.faces[i].v_idx]/2.0f;
        vertices[3*i+1] = attrib.vertices[3*attrib.faces[i].v_idx+1]/2.0f;
        vertices[3*i+2] = attrib.vertices[3*attrib.faces[i].v_idx+2]/2.0f;
    }
    *num_vertices = 3*attrib.num_faces;
    *vertices_ = vertices;
}
