#pragma once

#include "tmath.h"
#include <map>
#include <vector>
#include <list>
#include "TTriangleMesh.h"



//column �c�̗�
//row    ���̍s
/*------------------------------------------------------------------------------------
TSparseMatrix �a�s��N���X

�e�s(Row)���Ƃ� " list< int(column Idx), double (value)> m_M " �������� (0�v�f�����ɂ͉����z�u���Ȃ�)
 + row major form �ƌĂ΂��

 - �g���� - 
1) TSparseMatrix M( N, M)      �C���X�^���X�쐬���ɃT�C�Y���w��

2) �s��̒��g�����
   2-1) M.m_M.push_back( i,j, val ) (i,j)������ val ��}�� ( !����! �e�s�ɍ�����l�߂Ă����悤�ɂ���) 
   2-2) Laplacian �s��쐬�֐��Ȃǂ�����

3) createCompRowForm ���Ă�� ���`���������������߂� Field(m_Ap_c/m_Ai_c/m_Ax_c)�����
   Compressed Row Form�쐬�ȍ~�́AaddToExistEngry �Ȃǂ̗v�f�ɑ΂��鑀��́ACompressed Row Form�@field�ɑ΂��Ă��s����B

4) �Ō�ɘA��������������

���� �I�I
���̃N���X�� vector<> deque<> �̗v�f�Ƃ��Ă͎g��Ȃ�
Umfpack���g���Ă���֌W�� �[���R�s�[�������ł��Ȃ�����


--------------------------------------------------------------------------------------*/
class TSparseMatrix
{
public :

private:
	const int m_sizeRow; //Row     size
	const int m_sizeCol; //collumn size

	static HINSTANCE m_umgpackDll_hInst;
	static bool      m_bUmfpackDllAbailable;

	// matrix �{��-------------------------------------------------------------------- 
	typedef list< pair<int, double> > MatRow;
	typedef vector<MatRow>            Matrix;
	Matrix  m_M;

	//Compressed Row form (createCompressedRowForm���ĂԂ��Ƃō���)-----------------
	int      *m_Ap_c, *m_Ai_c; // note that m_Ap_c[ RosSize ] is the non zero nomber
	double   *m_Ax_c;

	//umfpack�𗘗p����LU��������ۂ�field--------------------------------------------
	void *m_umfPackSymbolic, *m_umfPackNumeric;

	//takapack�𗘗p����LU��������ۂ�field-------------------------------------------
	int      *m_LUp, *m_LUi, *m_LU_rowFlip; // note that m_Ap_c[ RosSize ] is the non zero nomber
	double   *m_LUx;


public:
	//contructors------------------------------------------------------------
	TSparseMatrix(int rowSize, int colSize );
	TSparseMatrix(const TSparseMatrix &src );
	~TSparseMatrix(void);

	//�ėp Interface--------------------------------------------------------
	void clear();
	void createCompRowForm( const int n_nonZeroEntry );// m_M --> m_Ap_c,m_Ai_c,m_Ax_c �����
	int  calcMmultMt ( TSparseMatrix &trgtMat                                              ) const; //trgtMat = M * Mt (M=this matrix) this function use "m_M" field
	void multVec     ( const int v_size, const double* v, const int trgt_size, double* trgt) const; //trgt    = M * v  this function uses "m_A*_c" field if available.
	void multVecToAdd( const int v_size, const double* v, const int trgt_size, double* trgt) const; //trgt   += M * v  this function uses "m_A*_c" field if available.
	void multScalar  ( double c);																    // M *= c 
	void createCopy( TSparseMatrix &trgt );

	inline void push_back( int i, int j, double val){ 
		assert( m_M[i].size() == 0 || m_M[i].back().first < j );
		m_M[i].push_back( pair<int,double>( j, val)); 
	}
	inline bool addToExistEngry( const int &i, const int &j, const double &val)
	{
		bool bIJexist = false;
		for( MatRow::iterator it = m_M[i].begin(); it != m_M[i].end(); ++it) if( it->first == j){ it->second += val; bIJexist = true; break; }
		if( !bIJexist   ) return false;
		if( m_Ap_c == 0 ) return true ;

		for( int idx = m_Ap_c[i]; idx < m_Ap_c[i+1]; ++idx) if( j == m_Ai_c[ idx ] ){ m_Ax_c[ idx ] += val; return true; }
		return false;
	}
	inline void add33BlockToExistEngry_onlyPointerField( const int &i, const int &j, const TMatrix9 &K){
		for( int idx = m_Ap_c[i  ]; idx < m_Ap_c[i+1]; ++idx) if( j== m_Ai_c[idx]){  m_Ax_c[ idx ] += K.data[0]; m_Ax_c[ idx+1] += K.data[3]; m_Ax_c[ idx+2] += K.data[6];}
		for( int idx = m_Ap_c[i+1]; idx < m_Ap_c[i+2]; ++idx) if( j== m_Ai_c[idx]){  m_Ax_c[ idx ] += K.data[1]; m_Ax_c[ idx+1] += K.data[4]; m_Ax_c[ idx+2] += K.data[7];}
		for( int idx = m_Ap_c[i+2]; idx < m_Ap_c[i+3]; ++idx) if( j== m_Ai_c[idx]){  m_Ax_c[ idx ] += K.data[2]; m_Ax_c[ idx+1] += K.data[5]; m_Ax_c[ idx+2] += K.data[8];}
	}
	inline void setAllEntryZero(){
		for( int i=0; i<m_sizeRow; ++i) 
		for( MatRow::iterator it = m_M[i].begin(); it != m_M[i].end(); ++it) it->second = 0;
		if( m_Ap_c != 0 ) memset( m_Ax_c, 0, sizeof( double ) * m_Ap_c[m_sizeRow] );
	}

	//umfpack�𗘗p���������� LU���� + ���^������������ ���s���Ƃ��́A�ȉ��̊֐����A�v���P�[�V�����̊J�n���ƏI�����ɌĂ�
	static bool load_UmfpackDll(const char *exeDirPath);
	static void free_UmfpackDll();

	//Laplacian (translated) Matrix creation-----------------------------------------------
	void calcLaplacianTransMat            (const TTriangleMesh &mesh);
	void calcLaplacianTransMat_fixBoudnary(const TTriangleMesh &mesh, const vector<short> &bBoundary, const vector<vector<int>> &vert_1RingVs);
	void calcLaplacianTransMat_fixBoudnary(const TTriangleMesh &mesh);

	void calcLaplacianTransMat_cotWeight_fixBoudnary(const TTriangleMesh &mesh);


	void calcLaplacianTransMat(    const   vector<TVector3> &vertices, const vector< vector< int > > &vert_1RingVs);
	void calcLaplacianTransMat(    const int vSize, TVector3*vertices, const vector< vector< int > > &vert_1RingVs);
	void calcLaplacianTransMat_cot( const TTriangleMesh &mesh, const byte *v_fix, const double FIX_COEF );


	//Matrix for Implicit integration of simple MASS-SPRING network------------------------
	void springModel_prepareEntries_3N_3N   (const int vSize, const vector<TWingEdge> &edges);
	void springModel_addSimpleSpring_dfdx   (const double stiffK, const double* verts, const vector<TWingEdge> &edges, const double *e_restLen);
	void springModel_addSimpleLaplacian_dfdx(const double stiffK, const vector<vector<int>>  &vert_1RingVs);

	//Linear System Solver ----------------------------------------------------------------
	void solveLinearSystem_CG(double* b, double* result ); //by CG method
	//Umfpack solver-----------------------------------------------------------------------
	void solveLinearSystem_LU_prepare();
	void solveLinearSystem_LU_solve  (const double *b, double *result);
	void solveLinearSystem_LU_free   ();

	void solveLinearSystem_LU(const double *b, double *result){
		solveLinearSystem_LU_prepare();
		solveLinearSystem_LU_solve(b,result);
		solveLinearSystem_LU_free();
	}
	//Trace & test matrix------------------------------------------------------------------
	void TraceMat_vector ();
	void TraceMat_pointer();
	static void TEST();
	static bool isUmfpackAvailable() { return m_bUmfpackDllAbailable; }

	int getSizeCol(){ return m_sizeCol; }
};






/* -----------class TDenseSqMat------------------------------------------------------------------
// LU�����̂��߂̖��s��N���X (Row Majour) (Row/Column flipping�@�\�t��)
//
// Pr:RowFlip / Pc:ColFlip�́A�s/�����ւ���\��
// TDenseSqMat M ����������ALU���������
// Pr M Pc = LU �ƂȂ�
// M : ���͍s��
// Pr:RowFlipMat(m_RowFlip) {0, 2, 1, 5, 4. 3}�Ȃ�@0,2,1,5,4,3�Ƃ�������row������Ƃ����Ӗ�
// Pc:ColFlipMat(m_ColFlip)
// LU: ���O�p/��O�p�s�� m_data�Ɋi�[�����
//
// !!!!!!!!!!!!�C���[�W!!!!!!!!! 
���s�� M �� Pr��Pc�Ńt���b�v�����s�� M' = Pr M Pc �� LU��������� m_data�ɓ����Ă��� 
LU������� i�s�ڂ� ���s�� M �� m_RowFlip[i]�s�� 
//
// 1 : �P����LU����
// 2 : �O�i����E��i����ɂ����`������������
// 3 : ����� n0 * n0 �����������ς݂̍s��ɑ΂���LU����
// 4 : Pr M Pc = LU �ƕ����ς݂̉��A
//     Pr( i, i+1 ) Pr M Pc' = L'U'�Ƃ����悤�ɁA�si, i+1�����ւ���LU����ۂs����ւ�
// 5 : �����ւ�
//
// ���T�|�[�g����
-----------class TDenseSqMat------------------------------------------------------------------*/
class TDenseSqMat
{
public:
	const int m_size   ;
	double  **m_data   ;
	int      *m_RowFlip;//Row flip Index
	int      *m_ColFlip;//Col flip Index 

	//constructors--------------------------------------------------------
	TDenseSqMat(int size);
	TDenseSqMat(const TDenseSqMat &src );
	~TDenseSqMat();

	inline int getSize()const{return m_size;}
	inline double*  operator[](int id) const { return m_data[id]; }

	//�ėp�@Interface ---------------------------------------------------
	inline void copy( const TDenseSqMat &src){ copy( src, m_size ); }
	inline void copy( const TDenseSqMat &src, int N){ //N*N�������R�s�[
		if( N > m_size || N > src.m_size ) return;
		for( int y = 0; y<N; ++y) memcpy( m_data[y], src.m_data[y], sizeof(double)*N);
		memcpy( m_RowFlip, src.m_RowFlip, sizeof( int  )*N);
		memcpy( m_ColFlip, src.m_ColFlip, sizeof( int  )*N);
	}
	
	inline void setSym(const int &i, const int &j, const double &v){ m_data[i][j]=m_data[j][i]=v; }
	inline void setZero(){ for( int i = 0; i<m_size; ++i) { m_RowFlip[i] = i; m_ColFlip[i] = i; memset( m_data[i], 0, sizeof(double)*m_size );} }
	inline void setIdentity(){
		setZero();
		for( int i = 0; i<m_size; ++i) m_data[i][i] = 1;
	}

	//LU-factorization (Crout's algorithm)------------------------------
	void LU_factorization_Full       (               const int *blockI, void (*progressFunc)(double) = 0 ); 
	void LU_factorization_RightBottom( const int n0, const int *blockI, void (*progressFunc)(double) = 0 ); 
	void LU_solveLinearSystem        ( const double *b, double *res                                      );

	//flip �A������row or column by Gondzio's algorithm
	bool LU_flipColumn               ( const int idx, const bool allowRowFlip);
	bool LU_flipRow                  ( const int idx, const bool allowColFlip);

	//Trace and Test -----------------------------------------------------
	void Trace        (int offset = 1);
	void Trace4       (int offset = 1);
	void TraceSymmetry(int offset = 1);
	void TraceSign    (int offset = 1);

	static void TEST();
	//TODO �|���Z���͖�����
};

































/*------------------------------------------------------------
TDenseMatrix ���s����N���X
m_rowSize �~ m_colSize�̖��s���\��

���`������solver������ Ax = b
LU����/QR����(�[���t�s��ɗ��p��)/umfpack/clapack
class TDenseMatrix
{
private:
	int m_rowSize;//�s�� (�c��)  transpose�̂Ƃ��ɕύX�����\������
	int m_colSize;//�� (����)
public:
	double **m_data;
	inline int getRowSize()const{return m_rowSize;}
	inline double*  operator[](int id) const { return m_data[id]; }

public:
	//Constructors-------------------------------------------------------------------------------------------//
	TDenseMatrix(int rowSize, int colSize);
	TDenseMatrix(const TDenseMatrix &src );
	~TDenseMatrix();

	//Set values---------------------------------------------------------------------------------------------//
	inline void setAllEntryZero(){
		for( int y = 0; y<m_rowSize; ++y)memset( m_data[y], 0, sizeof( double ) * m_colSize );
	}
	inline void copy( const TDenseMatrix &src){
		if(m_colSize != src.m_colSize || m_rowSize != src.m_rowSize ) return;
		for( int y = 0; y<m_rowSize; ++y) memcpy( m_data[y], src.m_data[y], sizeof( double ) * m_colSize );
	}
	inline void setIdentity(){
		for( int y = 0; y<m_rowSize; ++y) {
			memset( m_data[y], 0, sizeof( double ) * m_colSize );
			if( y < m_colSize )m_data[y][y] = 1;
		}
	}
	inline void transpose(){
		if( m_rowSize ==  m_colSize ){
			for( int y = 0  ; y<m_rowSize; ++y)
			for( int x = y+1; x<m_colSize; ++x) swap( m_data[x][y], m_data[y][x]); 
		}else{
			double **tmp = new double*[m_colSize]; 
			for( int i=0; i<m_colSize; ++i) tmp[i] = new double[ m_rowSize ];

			for( int y = 0  ; y<m_rowSize; ++y)
			for( int x = y+1; x<m_colSize; ++x) tmp[x][y] = m_data[y][x];
			swap( m_data, tmp );

			for( int i=0; i<m_rowSize; ++i) delete[] tmp[i];
			delete[] tmp;
			swap( m_rowSize, m_colSize);
		}
	}
	inline void multVector( const double *v, double *r ){ //r = Mv
		memset( r, 0, sizeof( double ) * m_rowSize );
		int idx = 0;
		for( int y = 0; y < m_rowSize; ++y )
		for( int x = 0; x < m_colSize; ++x ) r[y] += m_data[y][x] * v[x]; 
	}
	void setMatMultMat( TDenseMatrix &L, TDenseMatrix &R){
		if( L.m_colSize != R.m_rowSize || L.m_rowSize != m_rowSize || R.m_colSize != m_colSize) return;
		for(int y = 0; y<m_rowSize; ++y)
		for(int x = 0; x<m_colSize; ++x){
			double v =0;
			for( int k=0; k < L.m_colSize; ++k) v += L.m_data[y][k] * R.m_data[k][x];
			m_data[y][x] = v;
		}
	}
	//LU-factorization--------------------------------------
	void LU_factorization_Full       (               const int *blockI, int *rowFlip, void (*progressFunc)(double) = 0 ); 
	void LU_factorization_RightBottom( const int n0, const int *blockI, int *rowFlip, void (*progressFunc)(double) = 0 ); 
	void LU_SolveLinearSystem        ( const int *rowFlip, const double *b, double *res);
	void LU_matMult_dbg              ( const int *rowFlip, TDenseMatrix &trgt ); 

	//QR-factorization-------------------------------------
	void QR_factorization( TDenseMatrix &Q, TDenseMatrix &R)const ;//Q(Orthogonal) R(UpTriangle) are already allocated 

	void solveLinearSystem_QR     (       double *b, double *res )const ; //solve Ax = b by QR-factorization  (b-���������N����)
	void solveLinearSystem_LU     ( const double *b, double *res )const ; //solve Ax = b by LU-factorization

	void solveLinearSystem_umfpack( const double *b, double *res )const ; //solve Ax = b by umfpack
	void clapack_SolveLinearSystem( long* rowExchange, const double *b, double *res);
	void clapack_LU_factorization ( long* rowFlip  , long *rowExchange);

	//Trace & Test ----------------------------------------
	inline void Trace        (int offset = 1 );
	inline void Trace4       (int offset = 1 );
	inline void TraceSymmetry(int ofset  = 1 );
	inline void TraceSign    (int ofst   = 1 );

	static void TEST();
};
--------------------------------------------------------------*/










