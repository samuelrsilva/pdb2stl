/*
 * This is taken from the examples in libigl, so it is licensed under MPL2.
 */
#include <cstdio>
#include <igl/decimate.h>
#include <igl/readOFF.h>
#include <igl/writeOFF.h>

int main(int argc, char *argv[])
{
	int nfaces, p;
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;

	Eigen::MatrixXd U;
	Eigen::MatrixXi G;
	Eigen::VectorXi J;
	Eigen::VectorXi I;

	if (argc > 3) {
		p = atoi(argv[3]);
		if (p < 0 || p > 100) {
			p = 100;
		}

		if (!igl::readOFF(argv[1], V, F)) {
			std::fprintf(stderr, "Failed to read %s\n", argv[1]);
			return EXIT_FAILURE;
		}

		nfaces = F.rows() * p / 100.0;

		igl::decimate(V, F, nfaces, U, G, J, I);
		if (!igl::writeOFF(argv[2], U, G)) {
			std::fprintf(stderr, "Failed to write %s\n", argv[2]);
			return EXIT_FAILURE;
		}

	} else {
		std::printf("Usage: %s mesh.off decimated_mesh.off "
				"percentage of faces to keep\n", argv[0]);
	}

	return EXIT_SUCCESS;
}
