/*
 * off2stl.c
 *
 * Copyright Samuel Reghim Silva <samuelrsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define STRLEN 256

/* a point in cartesian coordinates */
struct cartesian {
	double x;
	double y;
	double z;
};

/* a triangle and its 3 vertices */
struct triangle {
	int a; /* indexes in a list of vertices */
	int b;
	int c;
};

/*
 * Read the triangular mesh from the given file in the Object File Format (OFF).
 * Memory of *triangles_p and *vertices_p must be freed by the caller.
 * Return 1 if successful, 0 otherwise.
 */
static int read_triangular_mesh(const char filenm[],
			struct triangle *triangles_p[], int *ntriangles_p,
			struct cartesian *vertices_p[], int *nvertices_p)
{
	FILE *f;
	char line[STRLEN];
	int i;

	/* list of all the vertices in the triangular grid */
	struct cartesian *vertices = NULL;
	int nvertices;
	double x, y, z;

	struct triangle *triangles = NULL;
	int ntriangles;
	int a, b, c;

	f = fopen(filenm, "r");
	if (f == NULL) {
		perror("Failed to open file for reading");
		return 0;
	}

	fgets(line, STRLEN, f); /* ignore first line (OFF) */

	fgets(line, STRLEN, f);
	sscanf(line, "%d %d", &nvertices, &ntriangles);
	if (nvertices < 0) {
		nvertices = 0;
	}
	if (ntriangles < 0) {
		ntriangles = 0;
	}
	printf("nvertices = %d, ntriangles = %d\n", nvertices, ntriangles);

	vertices = malloc((size_t)nvertices * sizeof(*vertices));
	if (vertices == NULL) {
		perror("Failed to allocate memory to read surface");
		fclose(f);
		return 0;
	}
	triangles = malloc((size_t)ntriangles * sizeof(*triangles));
	if (triangles == NULL) {
		perror("Failed to allocate memory to read surface");
		free(vertices);
		fclose(f);
		return 0;
	}

	i = 0;
	while (i < nvertices && fgets(line, STRLEN, f)) {
		if (sscanf(line, "%lf %lf %lf", &x, &y, &z) == 3) {
			vertices[i].x = x;
			vertices[i].y = y;
			vertices[i].z = z;
			++i;
		}
	}

	i = 0;
	while (i < ntriangles && fgets(line, STRLEN, f)) {
		if (sscanf(&line[1], "%d %d %d", &a, &b, &c) == 3) {
			if (a < 0 || a >= nvertices || b < 0 || b >= nvertices
			||  c < 0 || c >= nvertices) {
				fprintf(stderr, "Invalid input in %s: %s\n",
								filenm, line);
				fclose(f);
				free(vertices);
				free(triangles);
				return 0;
			}
			triangles[i].a = a;
			triangles[i].b = b;
			triangles[i].c = c;

			++i;
		}
	}

	fclose(f);

	*triangles_p = triangles;
	*ntriangles_p = ntriangles;

	*vertices_p = vertices;
	*nvertices_p = nvertices;

	return 1;
}

/*
 * Calculate the cross (or vector) product c = a X b
 */
static inline struct cartesian cross_product(const struct cartesian a,
						const struct cartesian b)
{
	struct cartesian c;

	c.x = a.y * b.z - a.z * b.y;
	c.y = a.z * b.x - a.x * b.z;
	c.z = a.x * b.y - a.y * b.x;

	return c;
}

/*
 * Calculate the dot (or scalar) product a.b
 */
static inline double dot_product(const struct cartesian a,
				 const struct cartesian b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

/*
 * ||v||
 */
static inline double vector_norm(const struct cartesian v)
{
	return sqrt(dot_product(v, v));
}

/*
 * Calculate the normal vector to the plane formed by a, b, c.
 */
static struct cartesian get_facet_normal(const struct cartesian a,
					const struct cartesian b,
					const struct cartesian c)
{
	struct cartesian ab, ac, n;
	double norm;

	ab.x = b.x - a.x;
	ab.y = b.y - a.y;
	ab.z = b.z - a.z;

	ac.x = c.x - a.x;
	ac.y = c.y - a.y;
	ac.z = c.z - a.z;

	n = cross_product(ab, ac);
	norm = vector_norm(n);
	n.x /= norm;
	n.y /= norm;
	n.z /= norm;

	return n;
}

int main(int argc, char *argv[])
{
	struct triangle *triangles = NULL;
	int ntriangles;

	struct cartesian *vertices = NULL;
	int nvertices;

	struct cartesian normal;

	int i;
	int a, b, c;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s mesh.off\n"
			"Prints the STL file to stdout\n", argv[0]);
		return EXIT_SUCCESS;
	}

	if (!read_triangular_mesh(argv[1], &triangles, &ntriangles,
						&vertices, &nvertices)) {
		fprintf(stderr, "Error reading molecular mesh\n");
		return EXIT_FAILURE;
	}
	fprintf(stderr, "%d triangles read\n", ntriangles);

	printf("solid vcg\n");

	for (i = 0; i < ntriangles; ++i) {
		a = triangles[i].a;
		b = triangles[i].b;
		c = triangles[i].c;

		normal = get_facet_normal(vertices[a], vertices[b],vertices[c]);
		printf(" facet normal % .6e % .6e % .6e\n",
						normal.x, normal.y, normal.z);
		printf("   outer loop\n");
		printf("     vertex  %.6e  %.6e  %.6e\n",
				vertices[a].x, vertices[a].y, vertices[a].z);
		printf("     vertex  %.6e  %.6e  %.6e\n",
				vertices[b].x, vertices[b].y, vertices[b].z);
		printf("     vertex  %.6e  %.6e  %.6e\n",
				vertices[c].x, vertices[c].y, vertices[c].z);
		printf("   endloop\n");
		printf(" endfacet\n");
	}

	printf("endsolid vcg\n");

	free(triangles);
	free(vertices);

	return EXIT_SUCCESS;
}
