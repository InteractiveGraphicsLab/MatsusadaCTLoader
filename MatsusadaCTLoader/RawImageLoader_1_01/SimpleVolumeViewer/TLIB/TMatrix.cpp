#include "StdAfx.h"
#include "TMatrix.h"
#include "takapack.h"
#include <Windows.h> 
#include <algorithm>
#include <string>


#include "umfpack.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef _WIN64 


#ifdef _DEBUG
#define TUmfpackDllName "UmfpackDll64d.dll"
#else
#define TUmfpackDllName "UmfpackDll64.dll"
#endif

#else

#ifdef _DEBUG
#define TUmfpackDllName "UmfpackDll32d.dll"
#else
#define TUmfpackDllName "UmfpackDll32.dll"
#endif

#endif



//umdpackの関数ポインタを宣言しておく
	
/*
	(void) umfpack_di_symbolic( m, n,   Ap_c, Ai_c, Ax_c, &Symbolic,           null, null) ;
	(void) umfpack_di_numeric (         Ap_c, Ai_c, Ax_c,  Symbolic, &Numeric, null, null) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap_c, Ai_c, Ax_c, result, d,  Numeric, null, null) ;//UMFPACK_At　そもそも対称なので、必要ない
	umfpack_di_free_symbolic( &Symbolic );
	umfpack_di_free_numeric ( &Numeric  );
*/


typedef void (WINAPI *T_UMFPACK_DI_SYMBOLIC)(int,int, const int*, const int*, const double*, void**);
typedef void (WINAPI *T_UMFPACK_DI_NUMERIC )(         const int*, const int*, const double*, void* , void**);
typedef void (WINAPI *T_UMFPACK_DI_SOLVE   )(int,     const int*, const int*, const double*, double*, const double*, void*);
typedef void (WINAPI *T_UMFPACK_DI_SYMBOLIC)(int,int, const int*, const int*, const double*, void**);
typedef void (WINAPI *T_UMFPACK_DI_FREE_SYMBOLIC)(void**);
typedef void (WINAPI *T_UMFPACK_DI_FREE_NUMERIC )(void**);

static T_UMFPACK_DI_SYMBOLIC      t_dll_umfpack_di_symbolic     ;
static T_UMFPACK_DI_NUMERIC       t_dll_umfpack_di_numeric      ;
static T_UMFPACK_DI_SOLVE         t_dll_umfpack_di_solve        ;
static T_UMFPACK_DI_FREE_SYMBOLIC t_dll_umfpack_di_free_symbolic;
static T_UMFPACK_DI_FREE_NUMERIC  t_dll_umfpack_di_free_numeric ;




bool      TSparseMatrix::m_bUmfpackDllAbailable = false;
HINSTANCE TSparseMatrix::m_umgpackDll_hInst     = 0;

bool TSparseMatrix::load_UmfpackDll(const char *exeDirPath)
{

//_AFXDLL が定義されている (＝共有DLLでMFCを使用)
#ifdef _AFXDLL
	string dllName = string(exeDirPath) + string( TUmfpackDllName );
	fprintf( stderr, "\n\n!!!!!!!!!!!!!load umfpack dll %s ......\n", dllName.c_str() );
	m_umgpackDll_hInst = AfxLoadLibrary( dllName.c_str() );	//DLLを読み込みます。
	if( m_umgpackDll_hInst == NULL ) return false;

	t_dll_umfpack_di_symbolic      = (T_UMFPACK_DI_SYMBOLIC      )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_symbolic"      );	//関数のアドレスを取得します。
	t_dll_umfpack_di_numeric       = (T_UMFPACK_DI_NUMERIC       )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_numeric"       );	//関数のアドレスを取得します。
	t_dll_umfpack_di_solve         = (T_UMFPACK_DI_SOLVE         )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_solve"         );	//関数のアドレスを取得します。
	t_dll_umfpack_di_free_symbolic = (T_UMFPACK_DI_FREE_SYMBOLIC )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_free_symbolic" );	//関数のアドレスを取得します。
	t_dll_umfpack_di_free_numeric  = (T_UMFPACK_DI_FREE_NUMERIC  )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_free_numeric"  );	//関数のアドレスを取得します。

	if( t_dll_umfpack_di_symbolic      == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_symbolic\n"     ); AfxFreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_numeric       == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_numeric\n"      ); AfxFreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_solve         == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_solve\n"        ); AfxFreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_free_symbolic == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_free_symbolic\n"); AfxFreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_free_numeric  == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_free_numeric\n" ); AfxFreeLibrary( m_umgpackDll_hInst ); return false; }

	m_bUmfpackDllAbailable = true;
	fprintf( stderr, "Done!!\n");
#else
	string dllName = string(exeDirPath) + string( TUmfpackDllName );
	fprintf( stderr, "\n\n!!!!!!!!!!!!!load umfpack dll %s ......\n", dllName.c_str() );
	m_umgpackDll_hInst = ::LoadLibrary( dllName.c_str() );	//DLLを読み込みます。
	if( m_umgpackDll_hInst == NULL ) return false;

	t_dll_umfpack_di_symbolic      = (T_UMFPACK_DI_SYMBOLIC      )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_symbolic"      );	//関数のアドレスを取得します。
	t_dll_umfpack_di_numeric       = (T_UMFPACK_DI_NUMERIC       )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_numeric"       );	//関数のアドレスを取得します。
	t_dll_umfpack_di_solve         = (T_UMFPACK_DI_SOLVE         )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_solve"         );	//関数のアドレスを取得します。
	t_dll_umfpack_di_free_symbolic = (T_UMFPACK_DI_FREE_SYMBOLIC )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_free_symbolic" );	//関数のアドレスを取得します。
	t_dll_umfpack_di_free_numeric  = (T_UMFPACK_DI_FREE_NUMERIC  )GetProcAddress( m_umgpackDll_hInst, "t_umfpack_di_free_numeric"  );	//関数のアドレスを取得します。

	if( t_dll_umfpack_di_symbolic      == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_symbolic\n"     ); ::FreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_numeric       == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_numeric\n"      ); ::FreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_solve         == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_solve\n"        ); ::FreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_free_symbolic == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_free_symbolic\n"); ::FreeLibrary( m_umgpackDll_hInst ); return false; }
	if( t_dll_umfpack_di_free_numeric  == NULL ){ fprintf( stderr, "fail : t_dll_umfpack_di_free_numeric\n" ); ::FreeLibrary( m_umgpackDll_hInst ); return false; }

	m_bUmfpackDllAbailable = true;
	fprintf( stderr, "Done!!\n");
#endif

	return true;
}

void TSparseMatrix::free_UmfpackDll(){

#ifdef _AFXDLL
	if( m_bUmfpackDllAbailable )  AfxFreeLibrary(m_umgpackDll_hInst);
#else
	if( m_bUmfpackDllAbailable )  ::FreeLibrary(m_umgpackDll_hInst);
#endif
}


/*
// Ax = b を解く   Aはcompressed row formで与えられる
// umfpackが利用可能なら umfpackを利用 利用不可ならtakapack利用
void t_SolveLinearSystem( int N, int *Ap_c, int* Ai_c, double* Ax_c, const double* d, double* result,double threshold );

void t_SolveLinearSystem( int N, int *Ap_c, int* Ai_c, double* Ax_c, const double* d, double* result,double threshold )
{
	double *null ;
	void *Symbolic, *Numeric ;

	null = (double *) NULL ;
	int m = N , n = N ;
	(void) umfpack_di_symbolic( m, n,   Ap_c, Ai_c, Ax_c, &Symbolic,           null, null) ;
	(void) umfpack_di_numeric (         Ap_c, Ai_c, Ax_c,  Symbolic, &Numeric, null, null) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap_c, Ai_c, Ax_c, result, d,  Numeric, null, null) ;//UMFPACK_At　そもそも対称なので、必要ない
	umfpack_di_free_symbolic( &Symbolic );
	umfpack_di_free_numeric ( &Numeric  );
}
*/




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//T_SPARSE_MATRIX//////////////////////////////////////////////////////////////////////////////////////////////////////
	
TSparseMatrix::~TSparseMatrix(void)
{
	clear(); 
}


void TSparseMatrix::clear()
{
	m_M.clear(); 
	m_M.resize( m_sizeRow );
	if( m_Ap_c != 0 ) delete[] m_Ap_c;   m_Ap_c = 0;
	if( m_Ai_c != 0 ) delete[] m_Ai_c;   m_Ai_c = 0;
	if( m_Ax_c != 0 ) delete[] m_Ax_c;   m_Ax_c = 0;

	if( m_LUp != 0 ) {
		takapack_LU_free( m_LUp, m_LUi, m_LUx, m_LU_rowFlip );
		m_LUp = m_LUi = m_LU_rowFlip = 0; // note that m_Ap_c[ RosSize ] is the non zero nomber
		m_LUx = 0;
	}

	if( m_bUmfpackDllAbailable && m_umfPackSymbolic != 0 ){
		t_dll_umfpack_di_free_symbolic (&m_umfPackSymbolic);   m_umfPackSymbolic = 0;
		t_dll_umfpack_di_free_numeric  (&m_umfPackNumeric );   m_umfPackNumeric  = 0;
	}
}


//createMatrix (n*m) rowSize * colSize
TSparseMatrix::TSparseMatrix (int rowSize, int colSize): m_sizeRow(rowSize), m_sizeCol(colSize)
{ 
	m_M.resize( m_sizeRow );  
	fprintf( stderr, "TSparseMatrix created %d %d\n", rowSize,colSize );
	m_Ap_c = 0;
	m_Ai_c = 0;
	m_Ax_c = 0;

	m_umfPackSymbolic = 0;
	m_umfPackNumeric  = 0;

	m_LUp = m_LUi = m_LU_rowFlip = 0; // note that m_Ap_c[ RosSize ] is the non zero nomber
	m_LUx = 0;


}
TSparseMatrix::TSparseMatrix( const TSparseMatrix &src ): m_sizeRow(src.m_sizeRow), m_sizeCol(src.m_sizeCol)
{
	for( int i=0; i<10; ++i) fprintf( stderr, "CAUTION NEVER USE THIS FUNCTION ");
	AfxMessageBox( "never user this function");
}




void TSparseMatrix::createCompRowForm( const int n_nonZeroEntry)
{
	if( m_Ap_c != 0 ) delete[] m_Ap_c;  m_Ap_c = new int   [ m_sizeRow + 1  ];//offset 1 が必要
	if( m_Ai_c != 0 ) delete[] m_Ai_c;  m_Ai_c = new int   [ n_nonZeroEntry ];
	if( m_Ax_c != 0 ) delete[] m_Ax_c;  m_Ax_c = new double[ n_nonZeroEntry ];

	int index = 0;
	for( int i = 0; i < m_sizeRow; ++i)
	{
		m_Ap_c[i] = index;
		for( MatRow::iterator it = m_M[i].begin(); it != m_M[i].end(); ++it )
		{
			m_Ai_c[index] = it->first;
			m_Ax_c[index] = it->second;
			++index;
		}
	}
	m_Ap_c[m_sizeRow] = index;
}


int TSparseMatrix::calcMmultMt( TSparseMatrix &trgtMat ) const
{
	//check
	if( trgtMat.m_sizeRow != m_sizeRow ){fprintf( stderr, "error 111123\n"); return 0;}  
	trgtMat.clear();
	Matrix& trgt = trgtMat.m_M;
	int zeroEntryNum = 0;
	int nonZeroNum = 0;


#pragma omp parallel for reduction( + : zeroEntryNum) , reduction( + : nonZeroNum )
	for( int i = 0; i < m_sizeRow ; ++i ){
		for( int j = 0; j < m_sizeRow ; ++j )
		{
			double d = 0;
			bool   bExistEntry = false;
			MatRow::const_iterator itI = m_M[i].begin();
			MatRow::const_iterator itJ = m_M[j].begin();
			while( itI != m_M[i].end() && itJ != m_M[j].end() ) 
			{
				if(      itI->first == itJ->first )
				{
					d += itI->second * itJ->second;
					bExistEntry = true;
					++itI; ++itJ;
				}
				else if( itI->first < itJ->first ) ++itI;
				else                               ++itJ;
			}
		
			if( !bExistEntry ) { zeroEntryNum++; continue;}
			trgt[i].push_back( pair< int, double >( j, d ) ); ++nonZeroNum;
		}
	}
	return nonZeroNum;
}



void TSparseMatrix::multVec( const int v_size, const double* v, const int trgt_size, double* trgt) const
{
	if( v_size != m_sizeCol || trgt_size != m_sizeRow ){ fprintf( stderr, "error 11123\n"); return ; }
	
	if( m_Ai_c == 0){ 
		for( int i = 0; i < (int) m_M.size(); ++i)
		{
			trgt[i] = 0;
			for( MatRow::const_iterator it = m_M[i].begin(); it != m_M[i].end(); ++it)  
				trgt[i] += v[ it->first ] * it->second;
		}
	}
	else
	{
		for( int i = 0; i < m_sizeRow; ++i)
		{
			trgt[i] = 0;
			for( int idx = m_Ap_c[i]; idx < m_Ap_c[i+1]; ++idx)
				trgt[i] += v[ m_Ai_c[idx] ] * m_Ax_c[idx];
		}
	}
}
void TSparseMatrix::multVecToAdd( const int v_size, const double* v, const int trgt_size, double* trgt) const
{
	if( v_size != m_sizeCol || trgt_size != m_sizeRow ){ fprintf( stderr, "error 11123\n"); return ; }
	
	if( m_Ai_c == 0){ 
		for( int i = 0; i < (int) m_M.size(); ++i)
		{
			for( MatRow::const_iterator it = m_M[i].begin(); it != m_M[i].end(); ++it)  
				trgt[i] += v[ it->first ] * it->second;
		}
	}
	else
	{
		for( int i = 0; i < m_sizeRow; ++i)
		{
			for( int idx = m_Ap_c[i]; idx < m_Ap_c[i+1]; ++idx)
				trgt[i] += v[ m_Ai_c[idx] ] * m_Ax_c[idx];
		}
	}
}

void TSparseMatrix::multScalar  ( double c){ //!注意!  m_A*_c が利用可能なときは m_Mにはかけ算は行われない
	if( m_Ax_c == 0 ) for( int i=0; i<m_sizeRow; ++i) for( MatRow::iterator it = m_M[i].begin(); it != m_M[i].end(); ++it) it->second *= c;
	else              for( int i=0; i<m_Ap_c[m_sizeRow]; ++i) m_Ax_c[i] *= c;	
}
	
void TSparseMatrix::createCopy( TSparseMatrix &trgt )
{
	if( m_sizeRow != trgt.m_sizeRow || m_sizeCol != trgt.m_sizeCol ){ fprintf( stderr, "cant Copy!!\n" ); return; }

	trgt.clear(); 
	for( int i = 0; i < m_sizeRow; ++i) trgt.m_M[i] = m_M[i];
	if( m_Ap_c != 0 ){
		int nonZeroEntry = m_Ap_c[m_sizeRow];
		trgt.m_Ap_c = new int   [ m_sizeRow + 1  ];  memcpy( trgt.m_Ap_c, m_Ap_c, sizeof(  int ) * ( m_sizeRow + 1 ) ); //offset 1 が必要
		trgt.m_Ai_c = new int   [ nonZeroEntry   ];  memcpy( trgt.m_Ai_c, m_Ai_c, sizeof(  int ) * nonZeroEntry      );
		trgt.m_Ax_c = new double[ nonZeroEntry   ];  memcpy( trgt.m_Ax_c, m_Ax_c, sizeof(double) * nonZeroEntry      );
	}
}

/*----------------------------------------------------------------------------
boundary頂点では, laplacianは計算せず xi = xi0を計算する (古い実装)
----------------------------------------------------------------------------*/
void TSparseMatrix::calcLaplacianTransMat_fixBoudnary(const TTriangleMesh       &mesh, 
													  const vector<short      > &bBoundary,
													  const vector<vector<int>> &vert_1RingVs)
{
	const int       vSize = mesh.getVnum();
	const TVector3 *verts = mesh.m_verts;
	if( m_sizeRow != vSize|| m_sizeCol != vSize) {fprintf( stderr, "error1211\n");}
	
	clear();
	for( int i = 0; i < vSize; ++i)
	{
		push_back( i, i, 1 );
		if( !bBoundary[i] )
		{
			double sizeInv = - 1.0 / (double) vert_1RingVs[i].size();
			for( int k=0; k<(int) vert_1RingVs[i].size(); ++k) push_back( vert_1RingVs[i][k], i, sizeInv);
		}
	}
}

/*----------------------------------------------------------------------------
boundary頂点では, laplacianは計算せず LtL[i][i] = 1とする
----------------------------------------------------------------------------*/
void TSparseMatrix::calcLaplacianTransMat_fixBoudnary(const TTriangleMesh &mesh)
{
	const int       vSize = mesh.getVnum();
	const TVector3 *verts = mesh.m_verts  ;
	if( m_sizeRow != vSize|| m_sizeCol != vSize) {fprintf( stderr, "error1211\n");}
	
	clear();
	forN( i , vSize)
	{
		push_back( i, i, 1 );
		if( ! mesh.bBoundaryVertex( i ) )
		{
			double sizeInv = - 1.0 / (double) mesh.m_v_1ring[i].size();
			forV( k, mesh.m_v_1ring[i]) push_back( mesh.m_v_1ring[i][k], i, sizeInv);
		}
	}
}








void TSparseMatrix::calcLaplacianTransMat_cotWeight_fixBoudnary(const TTriangleMesh &mesh)
{
	const int       vSize = mesh.getVnum();
	const TVector3 *verts = mesh.m_verts  ;
	if( m_sizeRow != vSize|| m_sizeCol != vSize) {fprintf( stderr, "error1211\n");}
	

	TVector3 d1,d2;

	clear();
	forN( vi , vSize)
	{
		if( mesh.bBoundaryVertex( vi ) ) { push_back( vi, vi, 1 ); continue; }

		const TVector3 &xi = verts[ vi ];

		vector<int> Vs, Ps, Es; 
		mesh.GetVsPsEsAroundVertex( vi, Vs, Ps, Es );


		double area = 0; //area
		forV( k, Ps ) area += mesh.GetPolygonArea( Ps[k] );
		area /= 3.0; //todo ボロノイ領域を計算する

		double sumWeight = 0;

		for( int k = 0, s = (int)Vs.size(); k < s; ++k)
		{
			const TVector3 &x0 = verts[ Vs[ k==0  ? s- 1 : k-1] ];
			const TVector3 &xj = verts[ Vs[                 k ] ];
			const TVector3 &x1 = verts[ Vs[ k==s-1? 0    : k+1] ];

			d1.SetSubtract( xi, x0); d1.Normalize_Self();
			d2.SetSubtract( xj, x0); d2.Normalize_Self();
			double a = t_getAngle_normalized_positive( d1,d2 );

			d1.SetSubtract( xi, x1); d1.Normalize_Self();
			d2.SetSubtract( xj, x1); d2.Normalize_Self();
			double b = t_getAngle_normalized_positive( d1,d2 );

			double wij = ( 1 / tan( a ) + 1 / tan( b ) ) / (area*2);

			sumWeight += wij;

			push_back( Vs[k], vi, -wij);

		}

		push_back( vi, vi, sumWeight );
	}

}










void TSparseMatrix::calcLaplacianTransMat(const vector<TVector3> &verts, const vector< vector< int > > &vert_1RingVs)
{
	if( m_sizeRow != (int) verts.size() || m_sizeCol != (int) verts.size() ) {fprintf( stderr, "error1211\n");}
	

	clear();

	for( int i = 0; i < (int) verts.size(); ++i)
	{
		push_back( i, i, 1 );
		bool oneRingContainSelf = false; // one ringが自分自身を含んでいる場合がある
		for(int j=0; j<(int) vert_1RingVs[i].size(); ++j )if( vert_1RingVs[i][j] == i){oneRingContainSelf = true; break;}
	
		double sizeInv = oneRingContainSelf ? - 1.0 / (double)( vert_1RingVs[i].size() -1):
			                                  - 1.0 / (double)  vert_1RingVs[i].size();	
		
		for(int j=0; j<(int) vert_1RingVs[i].size(); ++j )if( vert_1RingVs[i][j] != i) push_back( vert_1RingVs[i][j], i, sizeInv);
	}
}

void TSparseMatrix::calcLaplacianTransMat(const int vSize, TVector3* verts, const vector< vector< int > > &vert_1RingVs)
{
	if( m_sizeRow != vSize || m_sizeCol != vSize ) {fprintf( stderr, "error1211\n");}
	
	clear();
	for( int i = 0; i < vSize; ++i)
	{
		push_back( i, i, 1 );
		bool oneRingContainSelf = false; // one ringが自分自身を含んでいる場合がある
		for(int j=0; j<(int) vert_1RingVs[i].size(); ++j )if( vert_1RingVs[i][j] == i){oneRingContainSelf = true; break;}
	
		double sizeInv = oneRingContainSelf ? - 1.0 / (double)( vert_1RingVs[i].size() -1):
			                                  - 1.0 / (double)  vert_1RingVs[i].size();	
		
		for(int j=0; j<(int) vert_1RingVs[i].size(); ++j )if( vert_1RingVs[i][j] != i) push_back( vert_1RingVs[i][j], i, sizeInv);
	}
}

// cotangent weightを考慮した laplacian magrixを生成 (boundaryが有ったらダメ!!!!!)
// v_fix[vi] == true である頂点に対しては, M[vi][vi] に FIX_COEF を挿入する
void TSparseMatrix::calcLaplacianTransMat_cot( const TTriangleMesh &mesh, const byte *v_fix, const double FIX_COEF )
{
	const int vNum = mesh.getVnum();
	if( m_sizeRow != vNum || m_sizeCol != vNum ) {fprintf( stderr, "error1211\n");}
	clear();

	TVector3 *verts = mesh.m_verts;
	vector<int> Vs;

	TVector3 d1,d2;
	 //cotangent imple
	for( int vi = 0; vi < vNum; ++vi)
	{
		if( v_fix[ vi ] )
		{
			push_back( vi,vi, FIX_COEF );
		}
		else
		{
			Vs.clear();
			mesh.GetVsAroundVertex( vi, Vs );

			double sumWeight = 0;
			for( int j = 0, s = (int) Vs.size(); j < s; ++j)
			{
				int vJ     =             Vs[ j ];
				int vLeft  = (j !=0  ) ? Vs[j-1] : Vs.back ();
				int vRight = (j !=s-1) ? Vs[j+1] : Vs.front();
				d1.SetSubtract( verts[ vi ], verts[vLeft]); d1.Normalize_Self();
				d2.SetSubtract( verts[ vJ ], verts[vLeft]); d2.Normalize_Self();
				double a = t_getAngle_normalized_positive( d1,d2 );

				d1.SetSubtract( verts[ vi ], verts[vRight]); d1.Normalize_Self();
				d2.SetSubtract( verts[ vJ ], verts[vRight]); d2.Normalize_Self();
				double b = t_getAngle_normalized_positive( d1,d2 );

				double wij = 1 / tan( a ) + 1 / tan( b );
				//wij = 1;
				push_back( vJ, vi, wij);
				sumWeight += wij;
			}
			push_back( vi, vi, -sumWeight );
		}
	}
	/*
	for( int vi = 0; vi < vNum; ++vi)
	{
		if( v_fix[ vi ] ) push_back( vi,vi, FIX_COEF );
		else
		{
			Vs.clear(); mesh.GetVsAroundVertex( vi, Vs );
			
			for( int j = 0, s = (int) Vs.size(); j < s; ++j) push_back( Vs[ j ], vi, 1);
			push_back( vi, vi, -(double)Vs.size() );

		}
	}
	*/
}

//Ltを作る
void TSparseMatrix::calcLaplacianTransMat( const TTriangleMesh &mesh){
	calcLaplacianTransMat( mesh.getVnum(), mesh.m_verts, mesh.m_v_1ring);
}



//Matrix for Implicit integration of simple spring network 
/*--------------------------------------------------------------------------------------
Initialize 3N * 3N matrix
and its entries.

If edge vi-vj exist, this function set Kij(3*3) = {0,0,0  0,0,0  0,0,0}
This function also sets Kii = {0,0,0  0,0,0  0,0,0}
--------------------------------------------------------------------------------------*/
void TSparseMatrix::springModel_prepareEntries_3N_3N(const int vSize, const vector<TWingEdge> &edges)
{
	if( m_sizeCol != 3 * vSize ) return;
	if( m_sizeRow != 3 * vSize ) return;

	//ii
	for( int i=0; i<vSize; ++i){
		const int piv = 3*i;
		m_M[piv  ].push_back( make_pair(piv, 0)); m_M[piv  ].push_back( make_pair(piv+1, 0)); m_M[piv  ].push_back( make_pair(piv+2, 0));
		m_M[piv+1].push_back( make_pair(piv, 0)); m_M[piv+1].push_back( make_pair(piv+1, 0)); m_M[piv+1].push_back( make_pair(piv+2, 0));
		m_M[piv+2].push_back( make_pair(piv, 0)); m_M[piv+2].push_back( make_pair(piv+1, 0)); m_M[piv+2].push_back( make_pair(piv+2, 0));
	}

	//ij & ji
	for( int e=0; e<(int)edges.size(); ++e){
		const int pivI = 3 * edges[e].v[0];
		const int pivJ = 3 * edges[e].v[1];
		//Kij
		m_M[pivI  ].push_back( make_pair(pivJ, 0)); m_M[pivI  ].push_back( make_pair(pivJ+1, 0)); m_M[pivI  ].push_back( make_pair(pivJ+2, 0));
		m_M[pivI+1].push_back( make_pair(pivJ, 0)); m_M[pivI+1].push_back( make_pair(pivJ+1, 0)); m_M[pivI+1].push_back( make_pair(pivJ+2, 0));
		m_M[pivI+2].push_back( make_pair(pivJ, 0)); m_M[pivI+2].push_back( make_pair(pivJ+1, 0)); m_M[pivI+2].push_back( make_pair(pivJ+2, 0));
		//Kji
		m_M[pivJ  ].push_back( make_pair(pivI, 0)); m_M[pivJ  ].push_back( make_pair(pivI+1, 0)); m_M[pivJ  ].push_back( make_pair(pivI+2, 0));
		m_M[pivJ+1].push_back( make_pair(pivI, 0)); m_M[pivJ+1].push_back( make_pair(pivI+1, 0)); m_M[pivJ+1].push_back( make_pair(pivI+2, 0));
		m_M[pivJ+2].push_back( make_pair(pivI, 0)); m_M[pivJ+2].push_back( make_pair(pivI+1, 0)); m_M[pivJ+2].push_back( make_pair(pivI+2, 0));
	}


	class pairIntDoubleComp {
	public:
		bool operator()(const pair<int,double>& riLeft, const pair<int,double>& riRight) const {
			return riLeft.first < riRight.first; // ここでは、元の < の意味に合わせてあるが、異なる判断も可能。
		}
	};
	//convert map --> vector
	for( int i=0; i<m_sizeRow; ++i) m_M[i].sort( pairIntDoubleComp() );

	//debug用//
	//for( int i=0; i<m_sizeRow; ++i){
	//	for( MatRow::iterator it = m_M[i].begin(); it != m_M[i].end(); ++it) fprintf( stderr, "%d ", it->first);
	//	fprintf( stderr, "\n" );
	//}

	int nonZeroEntries = 9 * vSize + 9 * 2 * (int) edges.size();
	createCompRowForm( nonZeroEntries );
}




void TSparseMatrix::springModel_addSimpleSpring_dfdx   (const double stiffK, const double *verts, const vector<TWingEdge> &edges, const double *e_restLen)
{
	TMatrix9 Kij, Kii;
	TVector3 vij;
	for( int i=0; i<(int)edges.size(); ++i)
	{
		const TWingEdge &e = edges[i];
		const int idxI = e.v[0]*3;
		const int idxJ = e.v[1]*3;

		vij.data[0] = verts[idxJ  ] - verts[idxI  ]; 
		vij.data[1] = verts[idxJ+1] - verts[idxI+1]; 
		vij.data[2] = verts[idxJ+2] - verts[idxI+2]; 
		double L = vij.Length();

		double c =stiffK*( 1 - e_restLen[i]/L);
		Kij.data[0] = c; Kij.data[3] = 0; Kij.data[6] = 0;
		Kij.data[1] = 0; Kij.data[4] = c; Kij.data[7] = 0;
		Kij.data[2] = 0; Kij.data[5] = 0; Kij.data[8] = c;
		
		Kij.addMultVectors_withCoef( stiffK *e_restLen[i]/(L*L*L), vij, vij );
	
		Kii.Set( Kij );
		Kii.multScholar( -1 );

		add33BlockToExistEngry_onlyPointerField( idxI, idxI, Kii);
		add33BlockToExistEngry_onlyPointerField( idxJ, idxJ, Kii);
		add33BlockToExistEngry_onlyPointerField( idxI, idxJ, Kij);
		add33BlockToExistEngry_onlyPointerField( idxJ, idxI, Kij);//Kij_t=Kij
	}
}



void TSparseMatrix::springModel_addSimpleLaplacian_dfdx(const double stiffK, const vector<vector<int>>  &vert_1RingVs)
{

	TMatrix9 Kij, Kii;
	TVector3 vij;
	for( int i=0; i<(int)vert_1RingVs.size(); ++i)
	{
		const vector< int > &nei = vert_1RingVs[i];
		const int I = 3*i;
		addToExistEngry( I  ,I  , -stiffK * nei.size() );
		addToExistEngry( I+1,I+1, -stiffK * nei.size() );
		addToExistEngry( I+2,I+2, -stiffK * nei.size() );
		for( int k=0; k<(int)nei.size(); ++k) if( nei[k] != i)
		{
			const int J = 3*nei[k];
			addToExistEngry( I  ,J  , stiffK );
			addToExistEngry( I+1,J+1, stiffK );
			addToExistEngry( I+2,J+2, stiffK );
		}
	}
}


//Trace matrix--------------------------------------------------------------------------------------------------
void TSparseMatrix::TraceMat_vector(){
	fprintf( stderr, "\n\n!trace matrix(vector) row*col = %d %d\n", m_sizeRow, m_sizeCol );
	for( int i=0; i<m_sizeRow; ++i){
		fprintf( stderr, "row[%d] ", i);
		for( MatRow::iterator it= m_M[i].begin(); it != m_M[i].end(); ++it) fprintf( stderr, "[%d]%.1f ", it->first, it->second);
		fprintf( stderr, "\n");
	}
}
void TSparseMatrix::TraceMat_pointer(){
	fprintf( stderr, "\n\n!trace matrix(pointer) row*col = %d %d\n", m_sizeRow, m_sizeCol );
	if( m_Ax_c == 0) return;
	for( int i=0; i<m_sizeRow; ++i){
		fprintf( stderr, "row[%d] ", i);
		for( int k= m_Ap_c[i]; k < m_Ap_c[i+1]; ++k) fprintf( stderr, "[%d]%.1f, ", m_Ai_c[k], m_Ax_c[k]);
		fprintf( stderr, "\n");
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Linear System (CG)///////////////////////////////////////////////////////////////////////////////////////////
void TSparseMatrix::solveLinearSystem_CG(double* b, double* result )
{
	if( m_sizeRow != m_sizeCol) {fprintf( stderr, "error row size  is not equal to col size\n"); return;}
	if( m_Ai_c == 0           ) {fprintf( stderr, "error mat field is not ready!\n          "); return;}
	takapack_CG_sparse_solve( m_sizeRow, m_Ap_c, m_Ai_c, m_Ax_c, b, result, 0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Linear System (LU)///////////////////////////////////////////////////////////////////////////////////////////
void TSparseMatrix::solveLinearSystem_LU_prepare()
{
	if( m_sizeRow != m_sizeCol) {fprintf( stderr, "error row size is not equal to col size\n"); return;}
	if( m_Ai_c == 0           ) {fprintf( stderr, "error mat field is not ready!\n          "); return;}

	if( m_bUmfpackDllAbailable )
	{
		if( m_umfPackSymbolic != 0 ){
			t_dll_umfpack_di_free_symbolic (&m_umfPackSymbolic);  m_umfPackSymbolic = 0;
			t_dll_umfpack_di_free_numeric  (&m_umfPackNumeric );  m_umfPackNumeric  = 0;
		}
		int m = m_sizeCol , n = m_sizeCol ;
		t_dll_umfpack_di_symbolic( m, n, m_Ap_c, m_Ai_c, m_Ax_c, &m_umfPackSymbolic                   ) ;
		t_dll_umfpack_di_numeric (       m_Ap_c, m_Ai_c, m_Ax_c,  m_umfPackSymbolic, &m_umfPackNumeric) ;
	}
	else 
	{
		//takapack_ 利用
		if( m_LUp != 0 ) {
			takapack_LU_free( m_LUp, m_LUi, m_LUx, m_LU_rowFlip );
			m_LUp = m_LUi = m_LU_rowFlip = 0; // note that m_Ap_c[ RosSize ] is the non zero nomber
			m_LUx = 0;
		}
		takapack_LU_factorization( m_sizeRow, m_Ap_c, m_Ai_c, m_Ax_c, m_LUp, m_LUi, m_LUx, m_LU_rowFlip); 
	}
}


/* from umfpack.h------------------
#define UMFPACK_A	(0)	// Ax=b   
#define UMFPACK_At	(1)	// A'x=b   
#define UMFPACK_Aat	(2)	// A.'x=b  
----------------------------------*/
void TSparseMatrix::solveLinearSystem_LU_solve  (const double *b, double *result)
{
	if( m_sizeRow != m_sizeCol) {fprintf( stderr, "error row size is not equal to col size\n"); return;}
	if( m_Ai_c == 0           ) {fprintf( stderr, "error mat field is not ready!\n          "); return;}
	
	if( m_bUmfpackDllAbailable )
	{
		if( m_umfPackSymbolic == 0) solveLinearSystem_LU_prepare();
		(void) t_dll_umfpack_di_solve(UMFPACK_At, m_Ap_c, m_Ai_c, m_Ax_c, result, b, m_umfPackNumeric) ;//UMFPACK_At
	}else {
		takapack_LU_solve( m_sizeRow, m_LUp, m_LUi, m_LUx, m_LU_rowFlip, b, result); 
	}
}

void TSparseMatrix::solveLinearSystem_LU_free   ()
{
	if( m_bUmfpackDllAbailable )
	{
		if( m_umfPackSymbolic != 0 ){
			t_dll_umfpack_di_free_symbolic (&m_umfPackSymbolic); m_umfPackSymbolic = 0;
		    t_dll_umfpack_di_free_numeric  (&m_umfPackNumeric ); m_umfPackNumeric  = 0;
		}
	}else {
		if( m_LUp != 0 ) {
			takapack_LU_free( m_LUp, m_LUi, m_LUx, m_LU_rowFlip );
			m_LUp = m_LUi = m_LU_rowFlip = 0; // note that m_Ap_c[ RosSize ] is the non zero nomber
			m_LUx = 0;
		}
	}
}





void TSparseMatrix::TEST()
{
	fprintf( stderr, "\n\n\n\n!!!!!!!!!!!!!!!test TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr,         "!!!!!!!!!!!!!!!test TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");

	{   //単純な行列の場合 !
		//2  3  0 0 0    x0       8
		//3  0  4 0 6    x1      45
		//0 -1 -3 2 0 *  x2   = - 3    --> x = 1,2,3,4,5
		//0  0  1 0 0    x3       3
		//0  4  2 0 1    x4      19
		TSparseMatrix M( 5, 5);
		M.m_M[0].push_back( make_pair(0, 2.0));  M.m_M[0].push_back( make_pair(1, 3.0));
		M.m_M[1].push_back( make_pair(0, 3.0));                                           M.m_M[1].push_back( make_pair(2, 4.0));                               M.m_M[1].push_back( make_pair(4, 6.0));
													M.m_M[2].push_back( make_pair(1,-1.0));  M.m_M[2].push_back( make_pair(2,-3.0));  M.m_M[2].push_back( make_pair(3, 2.0));
																							M.m_M[3].push_back( make_pair(2, 1.0));  
													M.m_M[4].push_back( make_pair(1, 4.0));  M.m_M[4].push_back( make_pair(2, 2.0));                               M.m_M[4].push_back( make_pair(4, 1.0));
		M.createCompRowForm(12);
		const double b[5] = { 8., 45., -3., 3., 19.};
		double x1[5] = { 0, 0, 0, 0, 0};
		double x2[5] = { 0, 0, 0, 0, 0};

		// by LU factorization ( umfpack (if available) or takapack )-------------------
		M.solveLinearSystem_LU_prepare(     );
		M.solveLinearSystem_LU_solve  (b, x1);
		M.solveLinearSystem_LU_free   (     );
		//by takapack ------------------------------------------------------------------
		int    *LUi, *LUp, *LU_flip;
		double *LUx; 
		takapack_LU_factorization( 5, M.m_Ap_c,M.m_Ai_c,M.m_Ax_c, LUp, LUi, LUx, LU_flip);
		takapack_LU_solve        ( 5, LUp, LUi, LUx, LU_flip, b, x2 );
		takapack_LU_free         ( LUp, LUi, LUx, LU_flip );

		fprintf( stderr,"result(umf ) %f %f %f %f %f\n", x1[0], x1[1], x1[2], x1[3], x1[4] );
		fprintf( stderr,"result(taka) %f %f %f %f %f\n", x2[0], x2[1], x2[2], x2[3], x2[4] );
		
		double newb[5];
		M.multVec( 5, x1, 5, newb);
		fprintf( stderr,"new b %.2f %.2f %.2f %.2f %.2f (should be {8.45.-3,3,19})\n", newb[0], newb[1], newb[2], newb[3], newb[4] );

		const double t = 0.0000000001;
		bool  isFailed = false;
		if( fabs( x1[0]-1.0) > t  ||  fabs( x1[1]-2.0) > t  ||  fabs( x1[2]-3.0) > t  ||  fabs( x1[3]-4.0) > t  ||  fabs( x1[4]-5.0) > t ){ fprintf( stderr, "error1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); isFailed = true;}
		if( fabs( x2[0]-1.0) > t  ||  fabs( x2[1]-2.0) > t  ||  fabs( x2[2]-3.0) > t  ||  fabs( x2[3]-4.0) > t  ||  fabs( x2[4]-5.0) > t ){ fprintf( stderr, "error2!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); isFailed = true;}

		if( isFailed ) for( int i=0; i<10; ++i) fprintf( stderr, "!!!!!!!!!!!!!!!!!There are some errors in TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}

	{//対象行列の場合 (CG法が利用可能)	
	 //1  0  2  5    x0    1
	 //0 -4  1 -1  * x1  = 2
	 //2  1  0  2    x2    3
	 //5 -1  2  3    x3    4 
		TSparseMatrix A(4,4);
		A.m_M[0].push_back( make_pair(0, 1.0) );                                          A.m_M[0].push_back( make_pair(2, 2.0) );  A.m_M[0].push_back( make_pair(3, 5.0) );
												 A.m_M[1].push_back( make_pair(1,-4.0) ); A.m_M[1].push_back( make_pair(2, 1.0) );  A.m_M[1].push_back( make_pair(3,-1.0) );
		A.m_M[2].push_back( make_pair(0, 2.0) ); A.m_M[2].push_back( make_pair(1, 1.0) );                                           A.m_M[2].push_back( make_pair(3, 2.0) );
		A.m_M[3].push_back( make_pair(0, 5.0) ); A.m_M[3].push_back( make_pair(1,-1.0) ); A.m_M[3].push_back( make_pair(2, 2.0) );  A.m_M[3].push_back( make_pair(3, 3.0) );
		A.createCompRowForm(13);

		const double b[4] = { 1.0, 2.0, 3.0, 4.0};
		double x1[5] = { 0, 0, 0, 0, 0};
		double x2[5] = { 0, 0, 0, 0, 0};
		double x3[5] = { 0, 0, 0, 0, 0};
		// by LU factorization ( umfpack (if available) or takapack )-------------------
		A.solveLinearSystem_LU_prepare(     );
		A.solveLinearSystem_LU_solve  (b, x1);
		A.solveLinearSystem_LU_free   (     );
		//by takapack ------------------------------------------------------------------
		int    *LUi, *LUp, *LU_flip;
		double *LUx; 
		takapack_LU_factorization( 4, A.m_Ap_c,A.m_Ai_c,A.m_Ax_c, LUp, LUi, LUx, LU_flip);
		takapack_LU_solve        ( 4, LUp, LUi, LUx, LU_flip, b, x2 );
		takapack_LU_free         ( LUp, LUi, LUx, LU_flip );
		//by CG------------------------------------------------------------------------
		takapack_CG_sparse_solve( 4,  A.m_Ap_c,A.m_Ai_c,A.m_Ax_c, b, x3, 0.00000001 );

		fprintf( stderr,"result(umf ) %f %f %f %f \n", x1[0], x1[1], x1[2], x1[3] );
		fprintf( stderr,"result(taka) %f %f %f %f \n", x2[0], x2[1], x2[2], x2[3] );
		fprintf( stderr,"result(cg  ) %f %f %f %f \n", x3[0], x3[1], x3[2], x3[3] );

		bool  isFailed = false;
		const double t = 0.0000000001;
		if( fabs( x1[0]-x3[0]) > t  ||  fabs( x1[1]-x3[1]) > t  ||  fabs( x1[2]-x3[2]) > t  ||  fabs( x1[3]-x3[3]) > t   ){ fprintf( stderr, "error1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); isFailed = true;}
		if( fabs( x2[0]-x3[0]) > t  ||  fabs( x2[1]-x3[1]) > t  ||  fabs( x2[2]-x3[2]) > t  ||  fabs( x2[3]-x3[3]) > t   ){ fprintf( stderr, "error2!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"); isFailed = true;}

		if( isFailed ) for( int i=0; i<10; ++i) fprintf( stderr, "!!!!!!!!!!!!!!!!!There are some errors in TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

	}


	//to test other methods!!
	fprintf( stderr, "FIN FIN FIN test TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr, "FIN FIN FIN test TSparseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");



}









////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////TDenseQqMat////////////////////////////////////////////////////////////////////////////////////////////////
TDenseSqMat::TDenseSqMat(int size): m_size( size )
{
	fprintf(stderr, "TDenseSqMat construct %d \n", size );
	m_RowFlip = new int    [m_size]; 
	m_ColFlip = new int    [m_size]; 
	m_data    = new double*[m_size];

	for( int i = 0; i<m_size; ++i)
	{
		m_data[i] = new double[ m_size ];
		memset( m_data[i], 0, sizeof( double ) * m_size);
		m_RowFlip[i] = i; 
		m_ColFlip[i] = i;
	}
}

TDenseSqMat::TDenseSqMat(const TDenseSqMat &src ): m_size( src.m_size)
{
	fprintf(stderr, "TDenseSqMat copy construct %d\n", m_size, m_size  );
	
	m_RowFlip = new int    [m_size]; memcpy( m_RowFlip, src.m_RowFlip, sizeof( int  )*m_size);
	m_ColFlip = new int    [m_size]; memcpy( m_ColFlip, src.m_ColFlip, sizeof( int  )*m_size);
	m_data    = new double*[m_size];
	for( int i = 0; i<m_size; ++i) m_data[i] = new double[ m_size ];
	copy( src, m_size );
}

TDenseSqMat::~TDenseSqMat()
{
	fprintf(stderr, "TDenseSqMat destract %d\n", m_size);
	for( int y = 0; y<m_size; ++y) delete[] m_data[y];
	delete[] m_data   ;
	delete[] m_RowFlip;
	delete[] m_ColFlip;
}



/*--------------------------------------------------------
*LU_factorization_Full( const int *blockI, int *rowFlip)
*
*行列をLU分解する
*blockI       : block[i]の範囲でのみrow flipを行う
*               blockI = 0 なら全体がflipの対象になる
*progressFunc : 進捗を表示する関数ポインタ
*               null出なければ進捗に応じて[0,1]の値を入れる
*
--------------------------------------------------------*/
void TDenseSqMat::LU_factorization_Full( const int *blockI, void (*progressFunc)(double) )
{
	clock_t t0 = clock();

	const int N = m_size;
	for( int i=0;i<N; ++i) m_RowFlip[i] = i;

	double *Myi = new double[N];
	double *m;
	double big = 0, tmp; 
	int    piv = 0     ;

	for( int I = 0; I < N; ++I)
	{
#pragma omp parallel for
		for( int y=0; y < N; ++y) Myi[y] = m_data[y][I];//縦アクセスはこれ以上しない

		double v = 0;
		for( int y=0; y <=I; ++y) {//U and L
			v = Myi[y];
			for( int k = 0; k < y; ++k) v -= m_data[y][k] * Myi[k];
			Myi[y] = v;
		}
		big = fabs( Myi[I] );
		piv = I;

		for( int y=I+1; y < N; ++y) {//U and L
			v = Myi[y];//スタックメモリで高速にアクセスできるはず (ヒープは遅い?要実験)
			m = m_data[y];
			for( int k = 0; k < I; ++k, ++m) v -= *m * Myi[k];
			Myi[y] = v;
			if( ( ( blockI != 0 && y <= blockI[I]) || blockI == 0) && (tmp = fabs( Myi[y] )) > big ){ big = tmp;  piv = y;  }
		}

		if( piv != I ){
			swap( m_data   [I], m_data   [piv] );
			swap( m_RowFlip[I], m_RowFlip[piv] );
			swap(     Myi  [I],       Myi[piv] );
		}
		double coef = 1.0 / Myi[I];

#pragma omp parallel for
		for( int y = I+1; y < N; ++y) Myi[y] *= coef;
#pragma omp parallel for
		for( int y = 0  ; y < N; ++y) m_data[y][I] = Myi[y];

		if( I%100 == 0 && progressFunc != 0) progressFunc( I/(double)N );
	}
	delete[] Myi;

	fprintf( stderr, "LU factorization %.10f  \n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}



/*-------------------------------------------------------------------------
**LU_factorization_RightBottom( const int *blockI, int *rowFlip)
**
**ただし、n0 * n0の領域は分解済みの下、行列をLU分解する
**
**blockI       : block[i]の範囲でのみrow flipを行う
**                0なら全体がflipの対象になる
**progressFunc : 進捗を表示する関数ポインタ
**               null出なければ進捗に応じて[0,1]の値を入れる
**          n0  n1
**  M = n0 M11 M12  = 済 未
**      n1 M21 M22    未 未
--------------------------------------------------------*/
void TDenseSqMat::LU_factorization_RightBottom( const int n0, const int *blockI, void (*progressFunc)(double))
{
	clock_t t0 = clock();

	const int N = m_size;

	double *Myi = new double[N];
	for( int I = 0; I < n0; ++I)
	{
		double coef = 1.0 / m_data[I][I];

#pragma omp parallel for
		for( int y=0; y < N; ++y) Myi[y] = m_data[y][I];//縦アクセスはこれ以上しない

#pragma omp parallel for
		for( int xy = n0; xy < N; ++xy)
		{
			double a = 0;
			for( int k=0; k < I; ++k) a += m_data[xy][k] * Myi[k];
			m_data[xy][ I ] -= a   ; 
			m_data[xy][ I ] *= coef;
		}
	}

	double *m;
	double big = 0, tmp; 
	int    piv = 0     ;

	for( int I = n0; I < N; ++I)
	{
#pragma omp parallel for
		for( int y=0; y < N; ++y) Myi[y] = m_data[y][I];//縦アクセスはこれ以上しない

		double v = 0;
		for( int y=0; y <=I; ++y) {//U and L
			v = Myi[y];
			for( int k = 0; k < y; ++k) v -= m_data[y][k] * Myi[k];
			Myi[y] = v;
		}
		big = fabs( Myi[I] );
		piv = I;

		for( int y=I+1; y < N; ++y) {//U and L
			v = Myi[y];
			m = m_data[y];
			for( int k = 0; k < I; ++k, ++m) v -= *m * Myi[k];
			Myi[y] = v;
			if( ( blockI != 0 && y <= blockI[I]) && (tmp = fabs( Myi[y] )) > big ){ big = tmp;  piv = y;  }
		}
		if( piv != I ){
			swap( m_data   [I], m_data   [piv] );
			swap( m_RowFlip[I], m_RowFlip[piv] );
			swap(     Myi  [I],       Myi[piv] );
		}
		double coef = 1.0 / Myi[I];

#pragma omp parallel for
		for( int y = I+1; y < N; ++y) Myi[y] *= coef;
#pragma omp parallel for
		for( int y = 0  ; y < N; ++y) m_data[y][I] = Myi[y];

		if( I%100 == 0 && progressFunc != 0) progressFunc( I/(double)N );
	}
	delete[] Myi;

	fprintf( stderr, "LU factorization %.10f  \n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}



/*--------------------------------------------------------
**TDenseMatEx::LU_SolveLinearSystem( const double *b, double *res)
**LU分解がなされた下で、A res = bを解く
**--------------------------------------------------------*/
void TDenseSqMat::LU_solveLinearSystem( const double *b, double *res)
{
	clock_t t0 = clock();

	const int N = m_size	;
	double *f_B = new double[N];//fliped B
	double *f_Y = new double[N];//fliped Y
	double *f_X = new double[N];//fliped Y
	for( int i=0; i<N;++i) f_B[i] = b[ m_RowFlip[i] ];
	//前進代入 L*flipY = flipB --> a = L-1 b
	for( int y = 0; y < N; ++y){
		double v = f_B[y];
		for( int k=0; k<y; ++k) v -= m_data[y][k] * f_Y[k];
		f_Y[y] = v;
	}
	//solve U flipX = flipY (L-1 * flipB) 後退代入
	for( int y = N-1; y>=0; --y){
		double val = f_Y[y];
		for( int x = y+1; x < N; ++x) val -= m_data[y][x] * f_X[x];
		f_X[y] = val / m_data[y][y];
	}
	for( int i=0; i<N;++i) res[ m_ColFlip[i] ] = f_X[ i ];
	delete[] f_B;
	delete[] f_Y;
	delete[] f_X;
	fprintf( stderr, "forward/backward substitution %.10f sec\n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}


/*--------------------------------------------------------
**TDenseMatEx::LU_flipColumn( const int idx, const bool allowRowFlip, bool &doRowFlip)
**
** M'= Pr M Pc = LU 分解がされた下で、LU性を損なわないように、idx列目とidx+1列目をflipする。
**　その時に idx行目とidx+1行目をflipした方が数値的に安定であればflipする．
**
**    allowRowFlip : trueでかつ行flipを行った方が安定であれば行flipする
**    allowRowFlip = falseの場合、ゼロ割により計算できない事がある
　　　　　　　　　　 その場合falsを返す
**--------------------------------------------------------*/
bool TDenseSqMat::LU_flipColumn( const int idx, const bool allowRowFlip )
{
	// L = 1 0  U = a c 
	//     z 1      0 d
	const int N = m_size;
	if( idx >= N-1 ) { fprintf( stderr, "strange input at t_flipRow\n"); return false; }
	
	double a = m_data[idx  ][idx], c = m_data[idx  ][idx+1];
	double z = m_data[idx+1][idx], d = m_data[idx+1][idx+1];
	double v = d + z*c ;

	if( !allowRowFlip && fabs( c ) < 0.0000001 ) {fprintf( stderr, "error strange imput!!\n"); return false;}

	double entrySum1 = fabs(d/c) + 2;
	double entrySum2 = fabs( c/v) + 1 + fabs( d/v) + fabs( z);
	if( !allowRowFlip || ( fabs( c ) >= 0.00000001 && entrySum1 <= entrySum2) )
	{
		for( int i = 0; i < idx; ++i) swap( m_data[ i ][idx], m_data[ i ][idx+1] ); //x > y つまりUswap
		
		double m21 = d/c,  k21 = -d/c;

		m_data[idx  ][idx] = c  ;   m_data[idx  ][idx+1] =   a   ;
		m_data[idx+1][idx] = v/c;   m_data[idx+1][idx+1] = -a*d/c;

#pragma omp parallel for
		for( int i = idx+2; i < N; ++i){
			//L32 left / right  U23up / bottom
			double ll =       m_data[i][idx] + m_data[i][idx+1] * m21;
			double lr =                        m_data[i][idx+1]      ;
			double uu =       m_data[idx][i]               ;
			double ub = k21 * m_data[idx][i] + m_data[idx+1][i];

			m_data[idx  ][i] = uu;//U23up
			m_data[idx+1][i] = ub;//U23Bottom
			m_data[i][idx  ] = ll;//L32left 
			m_data[i][idx+1] = lr;//U32right
		}
	}else{
		for( int i = 0; i < idx; ++i) {
			swap( m_data[idx][ i ], m_data[idx+1][ i   ] ); //x < y つまりLswap
			swap( m_data[ i ][idx], m_data[ i   ][idx+1] ); //x > y つまりUswap
		}
		double m11 = c/v           ,   k11 = z              ;  
		double m21 = d/v,  m22 = -z,   k21 = d/v, k22 = -c/v;
		m_data[idx  ][idx] = v  ;   m_data[idx  ][idx+1] =   a*z ;
		m_data[idx+1][idx] = c/v;   m_data[idx+1][idx+1] = a*d/v;

#pragma omp parallel for
		for( int i = idx+2; i < N; ++i)
		{
			//L32 left / right  U23up / bottom
			double ll = m_data[i][idx] * m11 + m_data[i][idx+1] * m21;
			double lr = m_data[i][idx]       + m_data[i][idx+1] * m22;
			double uu = k11 * m_data[idx][i] +       m_data[idx+1][i];
			double ub = k21 * m_data[idx][i] + k22 * m_data[idx+1][i];
			m_data[idx  ][i] = uu;//U23up
			m_data[idx+1][i] = ub;//U23Bottom
			m_data[i][idx  ] = ll;//L32left 
			m_data[i][idx+1] = lr;//U32right
		}
		swap( m_RowFlip[ idx ], m_RowFlip[ idx+1 ] ); 
	}
	swap( m_ColFlip[ idx ], m_ColFlip[ idx+1 ] );
	return true;
}

//上の行flip版
bool TDenseSqMat::LU_flipRow( const int idx, const bool allowColFlip )
{
	// L = 1 0  U = a c 
	//     z 1      0 d
	const int N = m_size;
	if( idx >= N-1 ) { fprintf( stderr, "strange input at t_flipRow\n"); return false; }
	
	double a = m_data[idx  ][idx], c = m_data[idx  ][idx+1];
	double z = m_data[idx+1][idx], d = m_data[idx+1][idx+1];
	double v = d + z*c ;

	if( !allowColFlip && fabs( z ) < 0.00000001 ) return false;
	
	double entrySum1 = fabs( z  ) + fabs( 1/z) + 1;
	double entrySum2 = fabs( c/v) + 1 + fabs( d/v) + fabs( z);

	if( !allowColFlip || (fabs( z ) >= 0.000001 && entrySum1 <= entrySum2 ) )
	{
		for( int i = 0; i < idx; ++i) swap( m_data[idx][i], m_data[idx+1][i] );
		
		double m11 = 1/z           ,  k11 = z             ;
		double             m22 = -z,            k22 = -1/z;

		m_data[idx  ][idx] = a*z;   m_data[idx  ][idx+1] =   v ;
		m_data[idx+1][idx] = 1/z;   m_data[idx+1][idx+1] = -d/z;

#pragma omp parallel for
		for( int i = idx+2; i < N; ++i)
		{
			//L32 left / right  U23up / bottom
			double ll = m_data[i][idx] * m11                     ;
			double lr = m_data[i][idx]       + m_data[i][idx+1] * m22;
			double uu = k11 * m_data[idx][i] +       m_data[idx+1][i];
			double ub =                        k22 * m_data[idx+1][i];
			m_data[idx  ][i] = uu;//U23up
			m_data[idx+1][i] = ub;//U23Bottom
			m_data[i][idx  ] = ll;//L32left 
			m_data[i][idx+1] = lr;//U32right
		}
	}else{
		for( int i = 0; i < idx; ++i) {
			swap( m_data[idx][ i ], m_data[idx+1][ i   ] );
			swap( m_data[ i ][idx], m_data[ i   ][idx+1] );
		}
		double m11 = c/v           ,   k11 = z              ;  
		double m21 = d/v,  m22 = -z,   k21 = d/v, k22 = -c/v;
		m_data[idx  ][idx] = v  ;   m_data[idx  ][idx+1] =  a*z ;
		m_data[idx+1][idx] = c/v;   m_data[idx+1][idx+1] = a*d/v;

#pragma omp parallel for
		for( int i = idx+2; i < N; ++i)
		{
			//L32 left / right  U23up / bottom
			double ll = m_data[i][idx] * m11 + m_data[i][idx+1] * m21;
			double lr = m_data[i][idx]       + m_data[i][idx+1] * m22;
			double uu = k11 * m_data[idx][i] +       m_data[idx+1][i];
			double ub = k21 * m_data[idx][i] + k22 * m_data[idx+1][i];
			m_data[idx  ][i] = uu;//U23up
			m_data[idx+1][i] = ub;//U23Bottom
			m_data[i][idx  ] = ll;//L32left 
			m_data[i][idx+1] = lr;//U32right
		}
		swap( m_ColFlip[ idx ], m_ColFlip[ idx+1 ] );
	}
	swap( m_RowFlip[ idx ], m_RowFlip[ idx+1 ] ); 
	return true;
}


void TDenseSqMat::Trace(int offset ){
	fprintf( stderr, "-----TDenseMatEx Trace %d %d\n", m_size, m_size);
	for( int y = 0; y < m_size; y += offset ){
		for( int x = 0; x < m_size; x+= offset ) fprintf(stderr, "%.2f ",m_data[y][x] );
		fprintf( stderr,"\n");
	}
}
void TDenseSqMat::Trace4(int offset ){
	fprintf( stderr, "-----TDenseMatEx Trace %d %d\n", m_size, m_size);
	for( int y = 0; y < m_size; y += offset ){
		for( int x = 0; x < m_size; x+= offset ) fprintf(stderr, "%.4f ",m_data[y][x] );
		fprintf( stderr,"\n");
	}
}
void TDenseSqMat::TraceSymmetry(int ofset ){
	fprintf( stderr, "-----TDenseMatEx Trace %d %d\n", m_size, m_size);
	for( int y = 0; y < m_size; y += ofset ){
		for( int x = 0; x < m_size; x += ofset ) 
			if     ( m_data[y][x] == 0            ) fprintf( stderr, " 0");
			else if( m_data[y][x] == m_data[x][y] ) fprintf( stderr, " s");
			else                                    fprintf( stderr, " w");
		fprintf( stderr,"\n");
	}
}
void TDenseSqMat::TraceSign(int ofst ){
	fprintf( stderr, "-----TDenseMatEx Trace %d %d\n", m_size, m_size);
	for( int y = 0; y < m_size; y+=ofst ){
		for( int x = 0; x < m_size; x+=ofst ) 
			if     ( fabs( m_data[y][x])  <= 0.000001 ) fprintf( stderr, "0");
			else if( m_data[y][x] <0   ) fprintf( stderr, "-");
			else                         fprintf( stderr, "+");
		fprintf( stderr,"\n");
	}
}

void TDenseSqMat::TEST()
{
	fprintf( stderr, "\n\n\n\n-----------------TDenseMatEx::TEST()-----------------\n");
	fprintf( stderr,         "-----------------TDenseMatEx::TEST()-----------------\n");
	{
		//2  3  0  0  0
		//3  0  4  0  6
		//0 -1 -3  2  0
		//0  0  1  0  0
		//0  4  2  0  1
		TDenseSqMat M(5);
		M.m_data[0][0] = 2;  M.m_data[0][1] = 3;  M.m_data[0][2] = 0;  M.m_data[0][3] = 0;  M.m_data[0][4] = 0; 
		M.m_data[1][0] = 3;  M.m_data[1][1] = 0;  M.m_data[1][2] = 4;  M.m_data[1][3] = 0;  M.m_data[1][4] = 6; 
		M.m_data[2][0] = 0;  M.m_data[2][1] =-1;  M.m_data[2][2] =-3;  M.m_data[2][3] = 2;  M.m_data[2][4] = 0; 
		M.m_data[3][0] = 0;  M.m_data[3][1] = 0;  M.m_data[3][2] = 1;  M.m_data[3][3] = 0;  M.m_data[3][4] = 0; 
		M.m_data[4][0] = 0;  M.m_data[4][1] = 4;  M.m_data[4][2] = 2;  M.m_data[4][3] = 0;  M.m_data[4][4] = 1; 

		const double b[5] = { 8., 45., -3., 3., 19.};
		double x1[5] = { 0, 0, 0, 0, 0};
		M.LU_factorization_Full( 0, 0 );
		M.LU_solveLinearSystem(  b, x1);
		fprintf( stderr, "%f %f %f %f %f\n", x1[0],x1[1],x1[2],x1[3],x1[4] );

		const double t = 0.00000001;
		if( fabs( x1[0]-1) > t || fabs( x1[1]-2) > t || fabs( x1[2]-3) > t || fabs( x1[3]-4) > t || fabs( x1[4]-5) > t ) 
			for( int i =0; i < 10; ++i) fprintf( stderr, "!!!!!!!!!!!!error TDenseSqMat::TEST()!!!!!!!!!!!!!");
	}
	
	{
		//対象行列の場合 (CG法が利用可能)	
		//1  0  2  5    x0    1
		//0 -4  1 -1  * x1  = 2
		//2  1  0  2    x2    3
		//5 -1  2  3    x3    4 
		TDenseSqMat M(4);
		M[0][0] = 1;  M[0][1] = 0;  M[0][2] = 2;  M[0][3] = 5; 
		M[1][0] = 0;  M[1][1] =-4;  M[1][2] = 1;  M[1][3] =-1;  
		M[2][0] = 2;  M[2][1] = 1;  M[2][2] = 0;  M[2][3] = 2;  
		M[3][0] = 5;  M[3][1] =-1;  M[3][2] = 2;  M[3][3] = 3;  
		double b [4] = {1,2,3,4};
		double x1[4] = {0,0,0,0};
		double x2[4] = {0,0,0,0};
		takapack_CG_dense_solve( 4, M.m_data, b, x1 );
		M.LU_factorization_Full( 0, 0 );
		M.LU_solveLinearSystem( b, x2 );
		fprintf( stderr, "%f %f %f %f\n", x1[0],x1[1],x1[2],x1[3] );
		fprintf( stderr, "%f %f %f %f\n", x2[0],x2[1],x2[2],x2[3] );
		const double t = 0.00000001;
		if( fabs( x1[0]-x2[0]) > t || fabs( x1[1]-x2[1]) > t || fabs( x1[2]-x2[2]) > t || fabs( x1[3]-x2[3]) > t ) 
			for( int i =0; i < 10; ++i) fprintf( stderr, "!!!!!!!!!!!!error TDenseSqMat::TEST()!!!!!!!!!!!!!");
	}

	fprintf( stderr, "\nTDenseMatEx::TEST() done\n\n");
}










/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Dense matrix///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
TDenseMatrix::TDenseMatrix(int rowSize, int colSize): m_rowSize( rowSize ), m_colSize( colSize ){
	fprintf(stderr, "TDenseMatrix construct %d %d\n", m_rowSize,m_colSize );
	m_data = new double*[m_rowSize];
	for( int y = 0; y<m_rowSize; ++y){ m_data[y] = new double[ m_colSize ]; memset( m_data[y], 0, sizeof(double) * m_colSize );}
}

TDenseMatrix::TDenseMatrix(const TDenseMatrix &src ): m_rowSize( src.m_rowSize), m_colSize( src.m_colSize ){
	fprintf(stderr, "TDenseMatrix construct %d %d\n", m_rowSize,m_colSize );
	m_data = new double*[m_rowSize];
	for( int y = 0; y<m_rowSize; ++y){ m_data[y] = new double[ m_colSize ]; memcpy( m_data[y], src.m_data[y], sizeof( double ) * m_colSize ); }
}

TDenseMatrix::~TDenseMatrix(){
	fprintf(stderr, "TDenseMatrix destract %d %d\n", m_rowSize,m_colSize );
	for( int y = 0; y<m_rowSize; ++y) delete[] m_data[y];
	delete[] m_data;
}



//QR factorization by household method//
//Q: 直交行列   (allocated)
//R: 上三角行列 (allocated)
//note it's not necessary to compute Q when sloving Ax = b but compute Q^t b (こっちの方が明らかに軽い)
void TDenseMatrix::QR_factorization( TDenseMatrix &Q, TDenseMatrix &R)const 
{
	if( m_rowSize != Q.m_rowSize || m_rowSize != Q.m_colSize) return;
	if( m_rowSize != R.m_rowSize || m_colSize != R.m_colSize) return;
	if( m_rowSize < m_colSize ) return;

	R.copy( *this );
	Q.setIdentity();

	double *v   = new double [m_rowSize];
	double *vtR = new double [m_colSize];
	double *vtQ = new double [m_rowSize];

	for( int k = 0; k < m_colSize; ++k)
	{
		// H = |Ix   0    |    R' = HR   Q' = HQ  
		//     |0   I-vv^t|  
		const int n    = m_rowSize - k;
		double lenV_sq = 0;
		for( int i=0; i<n; ++i) { v[i] = R.m_data[k+i][k]; lenV_sq += v[i]*v[i];}
		double lenV = sqrt( lenV_sq );

		v[0] -= R.m_data[k][k]>0 ? -lenV : +lenV;

		//vを正規化
		lenV_sq += -R.m_data[k][k]*R.m_data[k][k] + v[0]*v[0];
		double coef = 2 / lenV_sq;

		//R' = H * R/////////////////////////////////////////////////////////////////
		for( int x = 0; x < m_colSize; ++x){
			vtR[x] = 0;
			for( int y=0; y<n; ++y) vtR[x] += v[y] * R.m_data[y+k][x];
			vtR[x] *= coef;
		}
		for( int y = 0; y < n        ; ++y) 
		for( int x = 0; x < m_colSize; ++x) R.m_data[y+k][x] -=  v[y] * vtR[x];

		//Q' = H * Q/////////////////////////////////////////////////////////////////
		for( int x = 0; x < m_rowSize; ++x){
			vtQ[x] = 0;
			for( int y=0; y<n; ++y) vtQ[x] += v[y] * Q.m_data[y+k][x];
			vtQ[x] *= coef;
		}
		for( int y = 0; y < n        ; ++y) 
		for( int x = 0; x < m_rowSize; ++x) Q.m_data[y+k][x] -=  v[y] * vtQ[x];
	}

	//今求めたQは  Hn Hn-1 ... H0 A = R --> QA = R というものなので A = Q^t R
	Q.transpose();

	delete[] v  ;
	delete[] vtR;
	delete[] vtQ;
}
*/

/*--------------------------------------------------
Solve A res = b by QR_factorization
double *b   //size:m_rowSize
double *res //size:m_colSize
--------------------------------------------------*/
/*
void TDenseMatrix::solveLinearSystem_QR( double *b, double *res )const 
{	
	clock_t aa = clock();
	double  *v    = new double [m_rowSize];
	double **R    = new double*[m_colSize];//転置した形で入れる

	for( int x=0; x<m_colSize; ++x){
		R[x] = new double[m_rowSize];
		for( int y=0; y<m_rowSize; ++y) R[x][y] = m_data[y][x];
	}
	//QR factorization/////////////////
	for( int k = 0; k < m_colSize; ++k)
	{
		// H = |Ix   0    |    R' = HR   Q' = HQ  
		//     |0   I-vv^t|  
		double lenV_sq = 0;
		for( int y=k; y<m_rowSize; ++y) { v[y] = R[k][y]; lenV_sq += v[y]*v[y];}//縦アクセス
		double lenV = sqrt( lenV_sq );
		v[k] -= R[k][k]>0 ? -lenV : +lenV; //v = x-y

		//vを正規化
		lenV_sq += -R[k][k] * R[k][k] + v[k]*v[k];  
		double coef = 2 / lenV_sq;

		//b' = H * b/////////////////////////////////////////////////////////////////
		double vt_b = 0;
		for( int y=k; y<m_rowSize; ++y) vt_b += v[y] * b[y];
		vt_b *= coef;	
		for( int y=k; y<m_rowSize; ++y) b[y] -= vt_b * v[y];

		//R' = H * R/////////////////////////////////////////////////////////////////
	    if( m_colSize - k < 100 ){
			for(int x = k; x < m_colSize; ++x){
				double vtR = 0;
				for(int y = k; y < m_rowSize; ++y) vtR += v[y] *R[x][y];//縦アクセス
				vtR *= coef;
				for(int y = k; y < m_rowSize; ++y) R[x][y] -= v[y] * vtR;//縦アクセス
			}
		}
		else{
			#pragma omp parallel shared( R, v)
			{
			#pragma omp for 
				for(int x = k; x < m_colSize; ++x){
					double vtR = 0;
					for(int y = k; y < m_rowSize; ++y) vtR += v[y] *R[x][y];//縦アクセス
					vtR *= coef;
					for(int y = k; y < m_rowSize; ++y) R[x][y] -= v[y] * vtR;//縦アクセス
				}
			}
		}
	}
	//solve R x = Qtb 後退代入
	for( int y = m_colSize-1; y>=0; --y)
	{
		double val = b[y];
		for( int x = y+1; x < m_colSize; ++x) val -= R[x][y] * res[x];//横アクセスになっちゃった・・・
		res[y] = val / R[y][y];
	}

	for( int i=0; i<m_rowSize; ++i)delete[] R[i];
	delete[] R   ;
	delete[] v   ;
	clock_t bb = clock();
	fprintf( stderr, "\nQR factorization%.10f\n\n", (bb-aa)/(double)CLOCKS_PER_SEC);
}


void TDenseMatrix::solveLinearSystem_LU( const double *b, double *res ) const
{
	if( m_colSize != m_rowSize) return;
	const int N = m_colSize ;
	int  *flipI = new int[ N ];
	for( int i=0;i<N; ++i) flipI[i] = i;

	clock_t t0 = clock();

	double big = 0, tmp, **A = m_data; 
	int piv = 0;
	for( int k=0; k<N; ++k) if( (tmp = fabs( A[k][0] ) ) > big ) { big = tmp; piv = k; } 

	if( piv != 0 ){
		swap(     A[0],     A[piv] );
		swap( flipI[0], flipI[piv] );
	}
	for( int k = 1; k< N; ++k) A[k][0] /= A[0][0];

	for( int I = 1; I < N; ++I )
	{
		double diagU = 0;
		for( int k=0; k < I; ++k) diagU += A[I][k] * A[k][I];
		A[I][I] -= diagU;

		//縦方向の計算と最大値検索を同時に
		big = fabs( A[I][I] ); piv = I;
#pragma omp parallel for
		for( int y = I+1; y < N; ++y)
		{
			double v = 0;
			for( int k=0; k < I; ++k) v += A[y][k] * A[k][I];
			A[y][I] -= v;

			if( (tmp = fabs( A[y][I] )) > big ){ big = tmp;  piv = y;  }
		}

		//縦方向(行) swap!
		if( piv != I ){ swap(     A[I],     A[piv] );
			            swap( flipI[I], flipI[piv] ); }

		double coef = 1.0 / A[I][I];
#pragma omp parallel for
		for( int y = I+1; y < N; ++y) A[y][I] *= coef;

		//横方向計算 (U)
#pragma omp parallel for
		for( int x = I+1; x < N; ++x){
			double v = 0;
			for( int k = 0; k < I; ++k) v -= A[I][k] * A[k][x];
			A[I][x] += v;
		}
	}

	clock_t t1 = clock();

	//L U x = b
	double *f_B = new double[N];//fliped B
	double *f_Y = new double[N];//fliped Y
	for( int i=0; i<N;++i) f_B[i] = b[ flipI[i] ];

	//前進代入 L*flipY = flipB --> a = L-1 b
	for( int y = 0; y < N; ++y){
		double v = f_B[y];
		for( int k=0; k<y; ++k) v -= A[y][k] * f_Y[k];
		f_Y[y] = v;
	}
	//solve U flipX = flipY (L-1 * flipB) 後退代入
	for( int y = N-1; y>=0; --y){
		double val = f_Y[y];
		for( int x = y+1; x < N; ++x) val -= A[y][x] * res[x];
		res[y] = val / A[y][y];
	}

	fprintf( stderr, "LU factorization %.10f  代入%.10f\n\n", (t1     -t0)/(double)CLOCKS_PER_SEC,
		                                                      (clock()-t1)/(double)CLOCKS_PER_SEC);
	delete[] flipI;
	delete[] f_B;
	delete[] f_Y;
}

void TDenseMatrix::solveLinearSystem_umfpack( const double *_b, double *res ) const 
{
	fprintf( stderr, "solve by umfpack %d %d\n", m_colSize, m_rowSize);
	if( m_colSize != m_rowSize ) return;
	if( m_colSize < 1 ) return;
	
	const int N = m_colSize;

	double *Ax_c = new double[ N * N ];
	int    *Ai_c = new int   [ N * N ];
	int    *Ap_c = new int   [ N +1  ];

	for( int y=0; y<N; ++y) Ai_c[y] = y;

	for( int y=0; y<N; ++y){
		Ap_c[y] = y * N;
		memcpy( &Ax_c[y*N], m_data[y], sizeof( double ) * N);
		memcpy( &Ai_c[y*N], &Ai_c [0], sizeof( int    ) * N);
	}
	Ap_c[N] = N*N;

	double *null ;
	void *Symbolic, *Numeric ;

	null = (double *) NULL ;
	int m = N , n = N ;
	(void) umfpack_di_symbolic( m, n,   Ap_c, Ai_c, Ax_c, &Symbolic,           null, null) ;
	(void) umfpack_di_numeric (         Ap_c, Ai_c, Ax_c,  Symbolic, &Numeric, null, null) ;
	(void) umfpack_di_solve (UMFPACK_A, Ap_c, Ai_c, Ax_c, res, _b,  Numeric, null, null) ;//UMFPACK_At　そもそも対称なので、必要ない
	umfpack_di_free_symbolic (&Symbolic);
	umfpack_di_free_numeric (&Numeric) ;
}

*/
/*--------------------------------------------------------
LU_factorization_Full( const int *blockI, int *rowFlip)
行列をLU分解する

blockI  : block[i]の範囲でのみrow flipを行う (もし0なら全体がflipの対象になる)
rowFlip : 選択された軸が入る
--------------------------------------------------------*/
/*
void TDenseMatrix::LU_factorization_Full( const int *blockI, int *rowFlip, void (*progressFunc)(double) )
{
	if( m_rowSize != m_colSize ) return;
	clock_t t0 = clock();
	
	const int N = m_rowSize;
	for( int i=0;i<N; ++i) rowFlip[i] = i;

	double *Myi = new double[N];
	double *m;
	double big = 0, tmp; 
	int    piv = 0     ;

	for( int I = 0; I < N; ++I)
	{
#pragma omp parallel for
		for( int y=0; y < N; ++y) Myi[y] = m_data[y][I];//縦アクセスはこれ以上しない

		double v = 0;
		for( int y=0; y <=I; ++y) {//U and L
			v = Myi[y];
			for( int k = 0; k < y; ++k) v -= m_data[y][k] * Myi[k];
			Myi[y] = v;
		}
		big = fabs( Myi[I] );
		piv = I;

		for( int y=I+1; y < N; ++y) {//U and L
			v = Myi[y];
			m = m_data[y];
			for( int k = 0; k < I; ++k, ++m) v -= *m * Myi[k];
			Myi[y] = v;
			if( ( blockI != 0 && y <= blockI[I]) && (tmp = fabs( Myi[y] )) > big ){ big = tmp;  piv = y;  }
		}
		if( piv != I ){
			swap(  m_data[I],  m_data[piv] );
			swap( rowFlip[I], rowFlip[piv] );
			swap(   Myi  [I],     Myi[piv] );
		}
		double coef = 1.0 / Myi[I];

#pragma omp parallel for
		for( int y = I+1; y < N; ++y) Myi[y] *= coef;
#pragma omp parallel for
		for( int y = 0  ; y < N; ++y) m_data[y][I] = Myi[y];

		if( I%100 == 0 && progressFunc != 0) progressFunc( I/(double)N );
	}
	delete[] Myi;

	fprintf( stderr, "LU factorization %.10f  \n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}

*/

/*--------------------------------------------------------
LU_factorization_RightBottom( const int *blockI, int *rowFlip)

行列をLU分解する
ただし、n0 * n0の領域は分解済みとする

blockI  : block[i]の範囲でのみrow flipを行う
          もし0なら全体がflipの対象になる

rowFlip : 選択された軸が入る

--------------------------------------------------------*/

/*
void TDenseMatrix::LU_factorization_RightBottom( const int n0, const int *blockI, int *rowFlip, void (*progressFunc)(double))
{
	clock_t t0 = clock();

	if( m_rowSize != m_colSize ) return;
	const int N = m_rowSize;

	//M21について (fN, 0)  (fN, fn)    M12について ( 0, 0)  (fN, fn)
	//M21について (fN, N)  ( N, fn)    M12について (fN, N)  ( N, fn)

	for( int I = 0; I < n0; ++I)
	{
		double coef = 1.0 / m_data[I][I];
#pragma omp parallel for
		for( int xy = n0; xy < N; ++xy)
		{
			double a = 0, b = 0;
			for( int k=0; k < I; ++k){ 
				a += m_data[xy][k] * m_data[k][ I ];
				b -= m_data[ I][k] * m_data[k][ xy];
			}
			m_data[xy][ I ] -= a; m_data[xy][ I ] *= coef;
			m_data[I ][xy ] += b;
		}
	}

	double big = 0, tmp; 
	int    piv = 0     ;
	for( int I = n0; I < N; ++I)
	{
		double diagU = 0;
		for( int k=0; k < I; ++k) diagU += m_data[I][k] * m_data[k][I];
		m_data[I][I] -= diagU;

		//縦方向の計算と最大値検索を同時に
		big = fabs( m_data[I][I] ); piv = I;
#pragma omp parallel for
		for( int y = I+1; y < N; ++y)
		{
			double v = 0;
			for( int k=0; k < I; ++k) v += m_data[y][k] * m_data[k][I];
			m_data[y][I] -= v;

			if( ( blockI != 0 && y <= blockI[I]) && (tmp = fabs( m_data[y][I] )) > big ){ big = tmp;  piv = y;  }
		}

		if( piv != I ){ //縦方向(行) swap!
			swap(  m_data[I],  m_data[piv] );
			swap( rowFlip[I], rowFlip[piv] );
		}

		double coef = 1.0 / m_data[I][I];

		//横方向計算 (U)
#pragma omp parallel for
		for( int x = I+1; x < N; ++x){
			m_data[x][I] *= coef;
			double v = 0;
			for( int k = 0; k < I; ++k) v -= m_data[I][k] * m_data[k][x];
			m_data[I][x] += v;
		}
		if( I%100 == 0 && progressFunc != 0) progressFunc( I/(double)N );
	}

	fprintf( stderr, "LU factorization %.10f todo todo todo 縦アクセス入れたほうが多分早い！ \n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}
*/

/*--------------------------------------------------------
LU_SolveLinearSystem( const int *rowFlip, const int *colFlip, const double *b, double res)
LU分解がなされた下で、A res = bを解く
--------------------------------------------------------*/
/*
void TDenseMatrix::LU_SolveLinearSystem( const int *rowFlip, const double *b, double *res)
{
	clock_t t0 = clock();

	const int N = m_rowSize	;
	double *f_B = new double[N];//fliped B
	double *f_Y = new double[N];//fliped Y
	double *f_X = new double[N];//fliped Y
	for( int i=0; i<N;++i) f_B[i] = b[ rowFlip[i] ];

	//前進代入 L*flipY = flipB --> a = L-1 b
	for( int y = 0; y < N; ++y){
		double v = f_B[y];
		for( int k=0; k<y; ++k) v -= m_data[y][k] * f_Y[k];
		f_Y[y] = v;
	}
	//solve U flipX = flipY (L-1 * flipB) 後退代入
	for( int y = N-1; y>=0; --y){
		double val = f_Y[y];
		for( int x = y+1; x < N; ++x) val -= m_data[y][x] * f_X[x];
		f_X[y] = val / m_data[y][y];
	}
	for( int i=0; i<N;++i) res[ i ] = f_X[ i ];
	delete[] f_B;
	delete[] f_Y;
	delete[] f_X;
	fprintf( stderr, "forward/backward substitution %.10f sec\n", (clock()-t0)/(double)CLOCKS_PER_SEC);
}

void TDenseMatrix::LU_matMult_dbg  ( const int *flipI, TDenseMatrix &trgt )
{
	const int N = m_colSize;

	for( int y = 0; y < N; ++y)
	for( int x = 0; x < N; ++x)
	{
		double v = 0;
		if( x == y ) {
			for( int k = 0; k < x; ++k) v += m_data[y][k] * 
				                             m_data[k][x];
			v += m_data[y][x];
		}else if( x > y ) {
			for( int k = 0; k < y; ++k) v += m_data[y][k] * 
				                             m_data[k][x];
			v += m_data[y][x];
		}else{
			for( int k = 0; k <=x; ++k) v += m_data[y][k] * 
				                             m_data[k][x];
		}

		trgt[y][x] = v;
	}
	double **tmp = new double*[N];
	for( int i=0; i< N; ++i) tmp[ flipI[i] ] = trgt.m_data[ i ];
	swap( trgt.m_data, tmp );
	delete[] tmp;
}

*/

/*********************************************************************************/
/*  int dgetrf_(integer *m, integer *n, doublereal *a, integer *lda,             */
/*              integer *ipiv, integer *info);                                   */
/*  一般行列Aを三角分解(LU分解)する関数，分解は置換Pを含んで行なわれる           */
/*  すなわち P*A=L*U (Lの対角要素は1)                                            */
/*-------------------------------------------------------------------------------*/
/*  引数   : *m    行列Aの行数                                                   */
/*         : *n    行列Aの列数                                                   */
/*         : *a    行列Aへのポインタ                                             */
/*         : *lda  行列Aの行数                                                   */
/*         : *ipiv ピボットの添字，詳しくは後述                                  */
/*         : *info 例外情報                                                      */
/*********************************************************************************/
//the matrix will be factorized into LU form (diagonal entlies of L are 1)  
/*
void TDenseMatrix::clapack_LU_factorization( long* rowIdces, long *rowExchange){
	clock_t t0 = clock();
	if( m_rowSize!= m_colSize ){ fprintf( stderr, "LU_fact_lapac is only for cubic matrix\n"); return; } 
	integer N = m_rowSize, INFO;

	//column majorなので入れ替え処理を行う必要がある
	double *m = new double[ N*N ];
	for( int y = 0; y<N; ++y)
	for( int x = 0; x<N; ++x) m[ y + x*N ] = m_data[y][x];

	dgetrf_( &N, &N, m, &N, rowExchange, &INFO );

	for( int y = 0; y<N; ++y)
	for( int x = 0; x<N; ++x) m_data[y][x] = m[ y + x*N ];


	for( int i=0; i< N; ++i) rowIdces[i] = i;
	for( int i=0; i< N; ++i) swap( rowIdces[i], rowIdces[ rowExchange[i]-1] );
	clock_t t1 = clock();
	fprintf( stderr, "LU factorization %.10f  \n", (t1-t0)/(double)CLOCKS_PER_SEC);
	delete[] m;
}
*/

/*********************************************************************************/
/* int dgetrs_(char *trans, integer *n, integer *nrhs, doublereal *a,            */
/*             integer *lda, integer *ipiv, doublereal *b, integer *ldb,         */
/*             integer *info);                                                   */
/*  LU分解済みのAについて線型の方程式系 AX = B の求解を行なう                    */
/*-------------------------------------------------------------------------------*/
/*  引数   : *trans 行列Aの転置の方程式系を求解する．                            */
/*                  転置なし: 'N'，転置: 'T'，共役転置: 'C'                      */
/*         : *n     行列Aの次元                                                  */
/*         : *nrhs  右辺の行列Bの列数                                            */
/*         : *a     行列Aへのポインタ                                            */
/*         : *lda   行列Aの行数                                                  */
/*         : *ipiv  ピボットの添字，分解のときに出力されたもの                   */
/*         : *b     行列Bへのポインタ                                            */
/*         : *ldb   行列Bの行数                                                  */
/*         : *info  例外情報                                                     */
/*********************************************************************************/
/*
void TDenseMatrix::clapack_SolveLinearSystem( long* rowExchange, const double *b, double *res)
{
	clock_t t0 = clock();
	if( m_rowSize!= m_colSize){ fprintf( stderr, "LU_fact_lapac is only for cubic matrix\n"); return; } 
	integer N = m_rowSize, N1 = 1, INFO;
	memcpy( res, b, sizeof( double ) * N );

	double *m = new double[ N*N ];
	for( int y = 0; y<N; ++y)
	for( int x = 0; x<N; ++x) m[ y + x*N ] = m_data[y][x];

	dgetrs_( "N", &N, &N1, m, &N, rowExchange, res, &N, &INFO );
	clock_t t1 = clock();
	fprintf( stderr, "前進行進代入 %.10f  \n", (t1-t0)/(double)CLOCKS_PER_SEC);
	delete m;
}
*/



/*

//Trace Matrix------------------------------------------------------------------------------------//
void TDenseMatrix::Trace(int offset){
	fprintf( stderr, "matSize %d %d\n", m_rowSize, m_colSize);
	for( int y = 0; y < m_rowSize; y += offset ){
		for( int x = 0; x < m_colSize; x+= offset ) fprintf(stderr, "%.2f ",m_data[y][x] );
		fprintf( stderr,"\n");
	}
}
void TDenseMatrix::Trace4(int offset){
	fprintf( stderr, "matSize %d %d\n", m_rowSize, m_colSize);
	for( int y = 0; y < m_rowSize; y += offset ){
		for( int x = 0; x < m_colSize; x+= offset ) fprintf(stderr, "%.4f ",m_data[y][x] );
		fprintf( stderr,"\n");
	}
}
void TDenseMatrix::TraceSymmetry(int ofset){
	fprintf( stderr, "matSize %d %d\n", m_rowSize, m_colSize);
	if( m_rowSize != m_colSize ) return;
	for( int y = 0; y < m_rowSize; y += ofset ){
		for( int x = 0; x < m_colSize; x += ofset ) 
			if     ( m_data[y][x] == 0            ) fprintf( stderr, " 0");
			else if( m_data[y][x] == m_data[x][y] ) fprintf( stderr, " s");
			else                                    fprintf( stderr, " w");
		fprintf( stderr,"\n");
	}
}
void TDenseMatrix::TraceSign(int ofst){
	fprintf( stderr, "\n\nmatSize %d %d-------------------------\n", m_rowSize, m_colSize);
	if( m_rowSize != m_colSize ) return;
	for( int y = 0; y < m_rowSize; y+=ofst ){
		for( int x = 0; x < m_colSize; x+=ofst ) 
			if     ( fabs( m_data[y][x])  <= 0.000001 ) fprintf( stderr, "0");
			else if( m_data[y][x] <0   ) fprintf( stderr, "-");
			else                         fprintf( stderr, "+");
		fprintf( stderr,"\n");
	}
}








void TDenseMatrix::TEST()
{
	fprintf( stderr, "\n\n\n\n\n\n");
	fprintf( stderr, "!!!!!!!!!!!!!!!test TDenseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr, "!!!!!!!!!!!!!!!test TDenseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");

	//solverに関するテスト
	const int N = 4;
	TDenseMatrix M( 4, 4 );
	M[0][0] = 1.0;  M[0][1] = 5.0;  M[0][2] = 9.0;  M[0][3] = 4.0; 
	M[1][0] = 2.0;  M[1][1] =11.0;  M[1][2] = 7.0;  M[1][3] =14.0; 
	M[2][0] = 3.0;  M[2][1] =10.0;  M[2][2] = 6.0;  M[2][3] =15.0; 
	M[3][0] =13.0;  M[3][1] = 2.0;  M[3][2] =12.0;  M[3][3] = 1.0; 

	double b[4] = {1, 2, 3, 4}, bqr[4] = {1, 2, 3, 4}, xlu[4], xqr[4] ;

	//LUで解く
	TDenseMatrix Mlu    ( M );
	Mlu.solveLinearSystem_LU( b, xlu );

	//QRで解く
	TDenseMatrix Q(4,4), R(4,4);
	M.solveLinearSystem_QR( bqr, xqr );

	fprintf( stderr, "lu v.s. qr %f %f  %f %f  %f %f  %f %f\n", xlu[0], xqr[0],  xlu[1], xqr[1],   xlu[2], xqr[2],   xlu[3], xqr[3] );
	for( int i=0; i<4; ++i) if( fabs( xlu[i] - xqr[i]) < 0.0000001 ) fprintf( stderr, "good!\n"); 
	                        else                                     fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!error1!!!!!\n");

	//CLapackで解く
	{
		long rFlip[4], rChange[4];
		double xclapack[4];
		TDenseMatrix Mlapack( M );
		Mlapack.clapack_LU_factorization( rFlip, rChange );

		Mlapack.Trace();

		Mlapack.clapack_SolveLinearSystem( rChange, b, xclapack );
		fprintf( stderr, "lu v.s. lapack %f:%f  %f:%f  %f:%f  %f:%f\n", xlu[0], xclapack[0],  xlu[1], xclapack[1],  
																		xlu[2], xclapack[2],  xlu[3], xclapack[3] );
		for( int i=0; i<4; ++i) 
			if( fabs( xlu[i] - xclapack[i]) < 0.0000001 ) fprintf( stderr, "good!\n"); 
			else                                          fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!error1!!!!!");
	}


	//LU分解に関するテスト
	TDenseMatrix Mnew( M ), Mtrgt(4,4);
	int flipI[4] = {0,1,2,3};
	Mnew.LU_factorization_Full(0, flipI,0);
	Mnew.LU_matMult_dbg( flipI, Mtrgt);


	Mtrgt.Trace();

	for( int y=0; y<4; ++y)
	for( int x=0; x<4; ++x) 
		if( fabs( Mtrgt[y][x] - M[y][x]) < 0.0000001 ) fprintf( stderr, "good!\n"); 
	    else                                           fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!error1!!!!!");
 
	{
		long rFlip[4], rChange[4];
		TDenseMatrix Mlapack( M ), Mres(4,4);
		Mlapack.clapack_LU_factorization( rFlip, rChange );

		int r[4] = {rFlip[0], rFlip[1], rFlip[2], rFlip[3]};
		Mlapack.LU_matMult_dbg( r, Mres);

		Mres.Trace();
		M   .Trace();
		for( int y=0; y<4; ++y)
		for( int x=0; x<4; ++x) 
			if( fabs( Mres[y][x] - M[y][x]) < 0.0000001 ) fprintf( stderr, "good!\n"); 
			else                                          fprintf( stderr, "!!!!!!!!!!!!!!!!!!!!error3 %d %d!!!!!\n", x,y);

	}

	fprintf( stderr, "!!!!!!!!!!!!!!!test TDenseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr, "!!!!!!!!!!!!!!!test TDenseMatrix!!!!!!!!!!!!!!!!!!!!!!\n");
	fprintf( stderr, "\n\n\n\n\n\n");
}



*/


