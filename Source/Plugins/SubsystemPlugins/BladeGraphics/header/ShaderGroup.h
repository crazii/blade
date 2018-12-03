/********************************************************************
	created:	2012/03/27
	filename: 	ShaderGroup.h
	author:		Crazii
	purpose:	
*********************************************************************/
#ifndef __Blade_MaterialSortGroup_h__
#define __Blade_MaterialSortGroup_h__
#include <utility/FixedArray.h>
#include <BladeGraphics.h>

namespace Blade
{
	class BLADE_GRAPHICS_API ShaderGroup : public Allocatable
	{
	public:
		//
		enum ERequirement
		{
			SR_BEGIN	= 0,
			SR_TEXTURE	= SR_BEGIN,	//texture property
			SR_ALPHABLEND,			//alpha blend property
			SR_FOG,					//fog property

			//
			SR_COUNT,
		};

		struct BLADE_GRAPHICS_API SRequireValue
		{
			union
			{
				struct  
				{
					uint8 mIndexStart;	//texture index
					uint8 mIndexEnd;
				};
				bool	mBool;			//render property switch
			};
			bool	mEnabled;

			SRequireValue()	{mEnabled = false;}
			SRequireValue(bool b) : mBool(b),mEnabled(true)											{}
			SRequireValue(uint8 start,uint8 end) :mIndexStart(start),mIndexEnd(end),mEnabled(true)	{}
			inline bool	isEnabled()	{return mEnabled;}
		};

	public:
		ShaderGroup();
		ShaderGroup(const TString& name);
		~ShaderGroup();

		/** @brief  */
		inline const TString&	getName() const		{return mName;}

		/** @brief  */
		SRequireValue			getRequirement(ERequirement eReq) const;

		/** @brief  */
		bool					setRequirement(ERequirement eReq, SRequireValue reqVal);

	protected:
		typedef SRequireValue RequirementList[SR_COUNT];

		RequirementList			mReqList;
		TString					mName;
	};//class ShaderGroup


	//////////////////////////////////////////////////////////////////////////
	//sort group list holding all sort groups
	//////////////////////////////////////////////////////////////////////////
	namespace Impl
	{
		class PassList;
		class ShaderGroupListImpl;
	}//namespace Impl

	class BLADE_GRAPHICS_API ShaderGroupList
	{
	public:
		ShaderGroupList();
		~ShaderGroupList();
		ShaderGroupList(const ShaderGroupList& src);
		ShaderGroupList& operator=(const ShaderGroupList& rhs);

		/** @brief  */
		void	clear();

		/** @brief  */
		void	reserve(size_t count);

		/** @brief  */
		index_t findGroup(const TString& name) const;

		/** @brief  */
		size_t	size() const;

		/** @brief  */
		void	push_back(const ShaderGroup& group);

		/** @brief  */
		ShaderGroup&	getAt(index_t index);

		/** @brief  */
		const ShaderGroup&	getAt(index_t index) const;

		inline ShaderGroup& operator[](index_t index)
		{
			return this->getAt(index);
		}

		inline const ShaderGroup& operator[](index_t index) const
		{
			return this->getAt(index);
		}
	private:
		Pimpl<Impl::ShaderGroupListImpl> mImpl;
	};//class ShaderGroupList

}//namespace Blade


#endif //__Blade_MaterialSortGroup_h__