#pragma once

#include "tmath.h"
#include <map>
#include <vector>
#include <list>
#include "TTriangleMesh.h"



//column 縦の列
//row    横の行
/*------------------------------------------------------------------------------------
TSparseMatrix 疎行列クラス

各行(Row)ごとに " list< int(column Idx), double (value)> m_M " を持つ実装 (0要素部分には何も配置しない)
 + row major form と呼ばれる

 - 使い方 - 
1) TSparseMatrix M( N, M)      インスタンス作成時にサイズを指定

2) 行列の中身を作る
   2-1) M.m_M.push_back( i,j, val ) (i,j)成分に val を挿入 ( !注意! 各行に左から詰めていくようにする) 
   2-2) Laplacian 行列作成関数などもある

3) createCompRowForm を呼んで 線形方程式を解くための Field(m_Ap_c/m_Ai_c/m_Ax_c)を作る
   Compressed Row Form作成以降は、addToExistEngry などの要素に対する操作は、Compressed Row Form　fieldに対しても行われる。

4) 最後に連立方程式を解く

注意 ！！
このクラスは vector<> deque<> の要素としては使わない
Umfpackを使っている関係で 深いコピーが実装できないため


--------------------------------------------------------------------------------------*/
class TSparseMatrix
{
public :

private:
	const int m_sizeRow; //Row     size
	const int m_sizeCol; //collumn size

	static HINSTANCE m_umgpackDll_hInst;
	static bool      m_bUmfpackDllAbailable;

	// matrix 本体-------------------------------------------------------------------- 
	typedef list< pair<int, double> > MatRow;
	typedef vector<MatRow>            Matrix;
	Matrix  m_M;

	//Compressed Row form (createCompressedRowFormを呼ぶことで作れる)-----------------
	int      *m_Ap_c, *m_Ai_c; // note that m_Ap_c[ RosSize ] is the non zero nomber
	double   *m_Ax_c;

	//umfpackを利用してLU分解する際のfield--------------------------------------------
	void *m_umfPackSymbolic, *m_umfPackNumeric;

	//takapackを利用してLU分解する際のfield-------------------------------------------
	int      *m_LUp, *m_LUi, *m_LU_rowFlip; // note that m_Ap_c[ RosSize ] is the non zero nomber
	double   *m_LUx;


public:
	//contructors------------------------------------------------------------
	TSparseMatrix(int rowSize, int colSize );
	TSparseMatrix(const TSparseMatrix &src );
	~TSparseMatrix(void);

	//汎用 Interface--------------------------------------------------------
	void clear();
	void createCompRowForm( const int n_nonZeroEntry );// m_M --> m_Ap_c,m_Ai_c,m_Ax_c を作る
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

	//umfpackを利用した高速な LU分解 + 線型方程式を解く を行うときは、以下の関数をアプリケーションの開始時と終了時に呼ぶ
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
// LU分解のための密行列クラス (Row Majour) (Row/Column flipping機能付き)
//
// Pr:RowFlip / Pc:ColFlipは、行/列入れ替えを表し
// TDenseSqMat M を初期化後、LU分解すると
// Pr M Pc = LU となる
// M : 入力行列
// Pr:RowFlipMat(m_RowFlip) {0, 2, 1, 5, 4. 3}なら　0,2,1,5,4,3という順でrowが入るという意味
// Pc:ColFlipMat(m_ColFlip)
// LU: 下三角/上三角行列 m_dataに格納される
//
// !!!!!!!!!!!!イメージ!!!!!!!!! 
元行列 M を PrとPcでフリップした行列 M' = Pr M Pc が LU分解されて m_dataに入っている 
LU分解後の i行目は 元行列 M の m_RowFlip[i]行目 
//
// 1 : 単純なLU分解
// 2 : 前進代入・後進代入による線形方程式を解く
// 3 : 左上の n0 * n0 成分が分解済みの行列に対するLU分解
// 4 : Pr M Pc = LU と分解済みの下、
//     Pr( i, i+1 ) Pr M Pc' = L'U'というように、行i, i+1を入れ替えつつLU性を保つ行入れ替え
// 5 : 列入れ替え
//
// をサポートする
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

	//汎用　Interface ---------------------------------------------------
	inline void copy( const TDenseSqMat &src){ copy( src, m_size ); }
	inline void copy( const TDenseSqMat &src, int N){ //N*N分だけコピー
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

	//flip 連続するrow or column by Gondzio's algorithm
	bool LU_flipColumn               ( const int idx, const bool allowRowFlip);
	bool LU_flipRow                  ( const int idx, const bool allowColFlip);

	//Trace and Test -----------------------------------------------------
	void Trace        (int offset = 1);
	void Trace4       (int offset = 1);
	void TraceSymmetry(int offset = 1);
	void TraceSign    (int offset = 1);

	static void TEST();
	//TODO 掛け算等は未実装
};

































/*------------------------------------------------------------
TDenseMatrix 密行列をクラス
m_rowSize × m_colSizeの密行列を表す

線形方程式solverを実装 Ax = b
LU分解/QR分解(擬似逆行列に利用可)/umfpack/clapack
class TDenseMatrix
{
private:
	int m_rowSize;//行数 (縦長)  transposeのときに変更される可能性あり
	int m_colSize;//列数 (横長)
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

	void solveLinearSystem_QR     (       double *b, double *res )const ; //solve Ax = b by QR-factorization  (b-書き換え起こる)
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










