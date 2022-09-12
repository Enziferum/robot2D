#include <robot2D/Graphics/Matrix3D.hpp>


namespace robot2D {
    Matrix3D::Matrix3D() {
        m_matrix[0] = 1.F; m_matrix[1] = 0.F; m_matrix[2] = 0.F; m_matrix[3] = 0.F;
        m_matrix[4] = 0.F; m_matrix[5] = 1.F; m_matrix[6] = 0.F; m_matrix[7] = 0.F;
        m_matrix[8] = 0.F; m_matrix[9] = 0.F; m_matrix[10] = 1.F; m_matrix[11] = 0.F;
        m_matrix[12] = 0.F; m_matrix[13] = 0.F; m_matrix[14] = 0.F; m_matrix[15] = 1.F;
    }

    Matrix3D::Matrix3D(float a0, float a1, float a2, float a3,
                       float a4, float a5, float a6, float a7,
                       float a8, float a9, float a10, float a11,
                       float a12, float a13, float a14, float a15) {
        m_matrix[0] = a0; m_matrix[1] = a1; m_matrix[2] = a2; m_matrix[3] = a3;
        m_matrix[4] = a4; m_matrix[5] = a5; m_matrix[6] = a6; m_matrix[7] = a7;
        m_matrix[8] = a8; m_matrix[9] = a9; m_matrix[10] = a10; m_matrix[11] = a11;
        m_matrix[12] = a12; m_matrix[13] = a13; m_matrix[14] = a14; m_matrix[15] = a15;
    }

    Matrix3D::Matrix3D(const float& value) {
        for (int it = 0; it < 16; ++it)
            m_matrix[it] = value;
    }

    Matrix3D Matrix3D::operator*(const Matrix3D& r) {
        Matrix3D m{0.f};

        for(short it = 0; it < 4; ++it){
            for(short ij = 0; ij < 4; ++ij){
                m(it, ij) = 0;
                for(short ik = 0; ik < 4; ++ik)
                    m(it, ij) += this ->operator()(it, ik) * r(ik, ij);
            }
        }

        return m;
    }

    Matrix3D Matrix3D::Identity{};

    vec3f Matrix3D::transformVector(const vec3f& rhs) const {
        return {
            m_matrix[0] * rhs.x + m_matrix[4] * rhs.y + m_matrix[8] * rhs.z + m_matrix[12],
            m_matrix[1] * rhs.x + m_matrix[5] * rhs.y + m_matrix[9] * rhs.z + m_matrix[13],
            m_matrix[2] * rhs.x + m_matrix[6] * rhs.y + m_matrix[10] * rhs.z + m_matrix[14]
        };
    }

    bool operator == (const Matrix3D& left, const Matrix3D& right) {
        for(int it = 0; it < 16; ++it)
            if(left[it] != right[it])
                return false;
        return true;
    }

    bool operator != (const Matrix3D& left, const Matrix3D& right) {
        return !(left == right);
    }

    vec3f operator* (const Matrix3D& left, const vec3f& vec) {
        return left.transformVector(vec);
    }

    Matrix3D Matrix3D::transpose() const {
        mat4 matrix{};
        for(int it=0; it < 4; ++it)
            for (int ij = 0; ij < 4; ++ij)
                matrix(ij, it) = this -> operator()(it, ij);
        return matrix;
    }
}