#include "cbase.h"
#include "iefx.h"
#include "decals.h"
#include "c_world.h"
#include "winlite.h"
#include "materialsystem/imaterialvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FDECAL_DYNAMIC 0x0100
#define FDECAL_IMMEDIATECLEANUP 0x8000

static class CRD_Decal_Particle_Manager : public CAutoGameSystem
{
public:
	void PostInit() override
	{
		// Deep in the decal init code, there's a function that merges overlapping decals.
		// The problem is, if we mark a decal for rendering only on the current frame,
		// it being merged with other decals will cause it to lose that flag.
		//
		// We're going to dive deep into the engine code and patch the function that checks
		// if a decal is overlapping other decals and make it ignore all overlaps where at
		// least one decal is flagged for immediate cleanup.

		// DecalColorShoot is the third (2'th) method in the vtable.
		const byte *pCreateDecal6 = reinterpret_cast< const byte *const *const * > ( effects )[0][2];

		// function starts at 1016d040
		// interesting call instruction is at 1016d0b0
		Assert( pCreateDecal6[0x70] == 0xe8 );
		const byte *pCreateDecal5 = pCreateDecal6 + 0x75 + *reinterpret_cast< const uintptr_t * >( &pCreateDecal6[0x71] );

		// function starts at 1016cae0
		// interesting call instruction is at 1016cb10
		Assert( pCreateDecal5[0x30] == 0xe8 );
		const byte *pCreateDecal4 = pCreateDecal5 + 0x35 + *reinterpret_cast< const uintptr_t * >( &pCreateDecal5[0x31] );

		// function starts at 1016c7b0
		// interesting call instruction is at 1016cac3
		Assert( pCreateDecal4[0x313] == 0xe8 );
		const byte *pCreateDecal3 = pCreateDecal4 + 0x318 + *reinterpret_cast< const uintptr_t * >( &pCreateDecal4[0x314] );

		// function starts at 1016c700
		// interesting call instruction is at 1016c77f
		Assert( pCreateDecal3[0x7f] == 0xe8 );
		const byte *pCreateDecal2 = pCreateDecal3 + 0x84 + *reinterpret_cast< const uintptr_t * >( &pCreateDecal3[0x80] );

		// function starts at 1016c4d0
		// interesting call instruction is at 1016c507
		Assert( pCreateDecal2[0x37] == 0xe8 );
		const byte *pCreateDecal1 = pCreateDecal2 + 0x3c + *reinterpret_cast< const uintptr_t * >( &pCreateDecal2[0x38] );

		// function starts at 1016c280
		// interesting call instruction is at 1016c4b0
		Assert( pCreateDecal1[0x230] == 0xe8 );
		const byte *pCreateDecal = pCreateDecal1 + 0x235 + *reinterpret_cast< const uintptr_t * >( &pCreateDecal1[0x231] );

		// function starts at 1016c0f0
		// interesting call instruction is at 1016c10c
		Assert( pCreateDecal[0x1c] == 0xe8 );
		byte *pFindOverlaps = const_cast< byte * >( pCreateDecal + 0x21 + *reinterpret_cast< const uintptr_t * >( pCreateDecal + 0x1d ) );

		// function starts at 1016aa30
		// need to replace two pieces (one 6 bytes, the other 5) of the function starting at 1016ab77
		//
		// OLD: 0f b7 47 54 a8 01 8b 4f 0c 0f 85 b1 02 00 00 a9 00 10 00 00 0f 85 a6 02 00 00
		// NEW: 66 f7 47 54 01 90 8b 4f 0c 0f 85 b1 02 00 00 f6 46 29 80 90 0f 85 a6 02 00 00
		Assert( pFindOverlaps[0x147] == 0x0f );
		Assert( pFindOverlaps[0x148] == 0xb7 );
		Assert( pFindOverlaps[0x149] == 0x47 );
		Assert( pFindOverlaps[0x14a] == 0x54 );
		Assert( pFindOverlaps[0x14b] == 0xa8 );
		Assert( pFindOverlaps[0x14c] == 0x01 );
		Assert( pFindOverlaps[0x14d] == 0x8b );
		Assert( pFindOverlaps[0x14e] == 0x4f );
		Assert( pFindOverlaps[0x14f] == 0x0c );
		Assert( pFindOverlaps[0x150] == 0x0f );
		Assert( pFindOverlaps[0x151] == 0x85 );
		Assert( pFindOverlaps[0x152] == 0xb1 );
		Assert( pFindOverlaps[0x153] == 0x02 );
		Assert( pFindOverlaps[0x154] == 0x00 );
		Assert( pFindOverlaps[0x155] == 0x00 );
		Assert( pFindOverlaps[0x156] == 0xa9 );
		Assert( pFindOverlaps[0x157] == 0x00 );
		Assert( pFindOverlaps[0x158] == 0x10 );
		Assert( pFindOverlaps[0x159] == 0x00 );
		Assert( pFindOverlaps[0x15a] == 0x00 );
		Assert( pFindOverlaps[0x15b] == 0x0f );
		Assert( pFindOverlaps[0x15c] == 0x85 );
		Assert( pFindOverlaps[0x15d] == 0xa6 );
		Assert( pFindOverlaps[0x15e] == 0x02 );
		Assert( pFindOverlaps[0x15f] == 0x00 );
		Assert( pFindOverlaps[0x160] == 0x00 );

		DWORD oldProtect{};
		VirtualProtect( pFindOverlaps + 0x147, 0x1a, PAGE_EXECUTE_READWRITE, &oldProtect );
		pFindOverlaps[0x147] = 0x66;
		pFindOverlaps[0x148] = 0xf7;
		pFindOverlaps[0x14b] = 0x01;
		pFindOverlaps[0x14c] = 0x90;
		pFindOverlaps[0x156] = 0xf6;
		pFindOverlaps[0x157] = 0x46;
		pFindOverlaps[0x158] = 0x29;
		pFindOverlaps[0x159] = 0x80;
		pFindOverlaps[0x15a] = 0x90;
		VirtualProtect( pFindOverlaps + 0x147, 0x1a, oldProtect, &oldProtect );
		FlushInstructionCache( GetCurrentProcess(), pFindOverlaps + 0x147, 0x1a );

		// Additionally, this version of Source Engine doesn't support immediate decal cleanup.
		// Maybe I should have mentioned that earlier. We're going to make it support it, though.
		// Unfortunately, there are four different functions that inline the function we need to change,
		// so we're going to have to patch four different functions.

		// DrawBrushModel is the first (0'th) method in the vtable.
		const byte *pEntryPoint4 = reinterpret_cast< const byte *const *const * > ( render )[0][2];

		// function starts at 1017b6d0
		// interesting call instruction is at 1017b6eb
		Assert( pEntryPoint4[0x1b] == 0xe8 );
		const byte *pEntryPoint3 = pEntryPoint4 + 0x20 + *reinterpret_cast< const uintptr_t * >( &pEntryPoint4[0x1c] );

		// function starts at 10142850
		// interesting call instruction is at 1014298d
		Assert( pEntryPoint3[0x13d] == 0xe8 );
		const byte *pEntryPoint2 = pEntryPoint3 + 0x13e + *reinterpret_cast< const uintptr_t * >( &pEntryPoint3[0x142] );

		// function starts at 10137c30
		// interesting call instruction is at 10137c48
		Assert( pEntryPoint2[0x18] == 0xe8 );
		const byte *pEntryPoint1 = pEntryPoint2 + 0x1d + *reinterpret_cast< const uintptr_t * >( &pEntryPoint2[0x19] );

		// function starts at 1016b970
		// interesting call instruction is at 1016bb61 (r_queued_decals 0)
		Assert( pEntryPoint1[0x1f1] == 0xe8 );
		const byte *pEntryPointA = pEntryPoint1 + 0x1f6 + *reinterpret_cast< const uintptr_t * >( &pEntryPoint1[0x1f2] );
		// interesting function pointer is at 1016bb29 (r_queued_decals 1)
		Assert( pEntryPoint1[0x1b8] == 0x68 );
		const byte *pEntryPointB = *reinterpret_cast< const byte *const * >( &pEntryPoint1[0x1b9] );

		// function starts at 1016a8a0
		// interesting call instruction is at 1016a8cd (r_drawbatchdecals 1)
		Assert( pEntryPointA[0x2d] == 0xe8 );
		byte *pDrawFuncA = const_cast< byte * >( pEntryPointA ) + 0x32 + *reinterpret_cast< const uintptr_t * >( &pEntryPointA[0x2e] );
		// interesting call instruction is at 1016a904 (r_drawbatchdecals 0)
		Assert( pEntryPointA[0x64] == 0xe8 );
		byte *pDrawFuncB = const_cast< byte * >( pEntryPointA ) + 0x69 + *reinterpret_cast< const uintptr_t * >( &pEntryPointA[0x65] );

		// function starts at 1016b870
		// interesting call instruction is at 1016b8b1 (r_drawbatchdecals 1)
		Assert( pEntryPointB[0x41] == 0xe8 );
		byte *pDrawFuncC = const_cast< byte * >( pEntryPointB ) + 0x46 + *reinterpret_cast< const uintptr_t * >( &pEntryPointB[0x42] );
		// interesting call instruction is at 1016b905 (r_drawbatchdecals 0)
		Assert( pEntryPointB[0x95] == 0xe8 );
		byte *pDrawFuncD = const_cast< byte * >( pEntryPointB ) + 0x9a + *reinterpret_cast< const uintptr_t * >( &pEntryPointB[0x96] );

		// All four of these functions have the same series of instructions we want to edit:
		// 4 bytes: get decal flags field
		// 5 bytes: check flag for dynamic decals
		// 2 bytes: bail if flag is not set
		// 5 bytes: check flag for updated this frame
		// 2 bytes: bail if flag is set
		// 5 bytes: get fade duration field
		// 5 bytes: set flag for updated this frame
		// 7 bytes: compare fade duration to floating point zero constant
		// 4 bytes: store updated flags
		// 2 bytes: bail if fade duration is less than or equal to zero
		// 5 bytes: call function to get client state
		// 2 bytes: store result as "this" pointer
		// 5 bytes: call function to compute current time
		// 3 bytes: load decal fade start time
		// 3 bytes: add decal fade duration
		// 2 bytes: swap registers for current time and decal fade end time
		// 2 bytes: compare decal fade end time with current time and pop floating point register stack
		// 2 bytes: pop floating point register stack
		// 2 bytes: bail if decal fade end time is after now
		// 3 bytes: compare decal's next cleanup pointer with 0
		// 6 bytes: jump out if the decal already has a next element in the cleanup list
		// 6 bytes: retrieve head of decal cleanup list
		// 3 bytes: store decal cleanup list in decal's next pointer
		// 6 bytes: store decal as new head of cleanup list
		// 5 bytes: jump out

		// We need to put the decal in the cleanup list if its "immediate cleanup" flag is set,
		// but NOT jump out of the loop iteration. We also need to keep all of the existing logic
		// and keep the same number of bytes of instructions.

		// TODO

		// function starts at 10169e40
		// relevant section starts at 1016a056
		pDrawFuncA;

		// function starts at 10168600
		// relevant section starts at 1016872c
		pDrawFuncB;

		// function starts at 1016aec0
		// relevant section starts at 1016afde
		pDrawFuncC;

		// function starts at 10167fd0
		// relevant section starts at 1016807b
		pDrawFuncD;
	}
} s_RD_Decal_Particle_Manager;

class C_OP_RenderDecal : public CParticleRenderOperatorInstance
{
	DECLARE_PARTICLE_OPERATOR( C_OP_RenderDecal );

	uint32 GetWrittenAttributes( void ) const override
	{
		return 0;
	}

	uint32 GetReadAttributes( void ) const override
	{
		return PARTICLE_ATTRIBUTE_XYZ_MASK | PARTICLE_ATTRIBUTE_TINT_RGB_MASK |
			PARTICLE_ATTRIBUTE_YAW_MASK | PARTICLE_ATTRIBUTE_NORMAL_MASK;
	}

	uint64 GetReadControlPointMask() const override
	{
		uint64 nMask = 0;
		if ( VisibilityInputs.m_nCPin >= 0 )
			nMask |= 1ULL << VisibilityInputs.m_nCPin;
		return nMask;
	}

	void Render( IMatRenderContext *pRenderContext, CParticleCollection *pParticles, const Vector4D &vecDiffuseModulation, void *pContext ) const override;

	struct C_OP_RenderDecalContext_t
	{
		CParticleVisibilityData m_VisibilityData;
		int m_iDecalIndex;
		bool m_bIsModel;
	};

	size_t GetRequiredContextBytes( void ) const override
	{
		return sizeof( C_OP_RenderDecalContext_t );
	}

	void InitializeContextData( CParticleCollection *pParticles, void *pContext ) const override
	{
		C_OP_RenderDecalContext_t *pCtx = reinterpret_cast< C_OP_RenderDecalContext_t * >( pContext );

		pCtx->m_VisibilityData.m_bUseVisibility = false;

		Assert( pParticles );
		Assert( pParticles->m_pDef );

		pCtx->m_iDecalIndex = effects->Draw_DecalIndexFromName( const_cast< char * >( pParticles->m_pDef->MaterialName() ) );

		IMaterial *pMaterial = pParticles->m_pDef->GetMaterial();
		Assert( pMaterial );

		static unsigned s_ModelVarCache = 0;
		IMaterialVar *pModelVar = pMaterial->FindVarFast( "$model", &s_ModelVarCache );
		pCtx->m_bIsModel = pModelVar && pModelVar->GetIntValueFast() != 0;
	}
};

DEFINE_PARTICLE_OPERATOR( C_OP_RenderDecal, "render_decal", OPERATOR_GENERIC );

void C_OP_RenderDecal::Render( IMatRenderContext *pRenderContext, CParticleCollection *pParticles, const Vector4D &vecDiffuseModulation, void *pContext ) const
{
	C_OP_RenderDecalContext_t *pCtx = reinterpret_cast< C_OP_RenderDecalContext_t * >( pContext );

	int nParticles;
	const ParticleRenderData_t *pRenderList = pParticles->GetRenderList( pRenderContext, true, &nParticles, &pCtx->m_VisibilityData );

	size_t xyz_stride, tint_stride, yaw_stride, normal_stride;
	const fltx4 *xyz = pParticles->GetM128AttributePtr( PARTICLE_ATTRIBUTE_XYZ, &xyz_stride );
	const fltx4 *tint = pParticles->GetM128AttributePtr( PARTICLE_ATTRIBUTE_TINT_RGB, &tint_stride );
	const fltx4 *yaw = pParticles->GetM128AttributePtr( PARTICLE_ATTRIBUTE_YAW, &yaw_stride );
	const fltx4 *normal = pParticles->GetM128AttributePtr( PARTICLE_ATTRIBUTE_NORMAL, &normal_stride );

	for ( int i = 0; i < nParticles; i++ )
	{
		int group = i / 4;
		int sub = i & 3;

		Vector origin{ SubFloat( xyz[xyz_stride * group], sub ), SubFloat( xyz[xyz_stride * group + 1], sub ), SubFloat( xyz[xyz_stride * group + 2], sub ) };
		color32 color{ FastFToC( SubFloat( tint[tint_stride * group], sub ) ), FastFToC( SubFloat( tint[tint_stride * group + 1], sub ) ), FastFToC( SubFloat( tint[tint_stride * group + 2], sub ) ), 255 };
		Vector up{ SubFloat( normal[normal_stride * group], sub ), SubFloat( normal[normal_stride * group + 1], sub ), SubFloat( normal[normal_stride * group + 2], sub ) };
		if ( up.IsZero() )
			up.z = 1;
		up.NormalizeInPlace();
		Vector forward = UTIL_YawToVector( SubFloat( yaw[yaw_stride * group], sub ) );

		Assert( !pCtx->m_bIsModel );
		effects->DecalColorShoot( pCtx->m_iDecalIndex, 0,
			GetClientWorldEntity()->GetModel(), vec3_origin, vec3_angle,
			origin, &forward, FDECAL_DYNAMIC | FDECAL_IMMEDIATECLEANUP,
			color, &up );
	}
}

BEGIN_PARTICLE_RENDER_OPERATOR_UNPACK( C_OP_RenderDecal )
END_PARTICLE_OPERATOR_UNPACK( C_OP_RenderDecal )

void RegisterRDDecalParticleOps()
{
	REGISTER_PARTICLE_OPERATOR( FUNCTION_RENDERER, C_OP_RenderDecal );
}
