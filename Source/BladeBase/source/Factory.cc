/********************************************************************
	created:	2010/08/10
	filename: 	Factory.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <Factory.h>

namespace Blade
{

	namespace FactoryUtil
	{
		class UniversalHRD : public HRD, public StaticAllocatable
		{
		public:
			UniversalHRD()	{}
			virtual ~UniversalHRD()	{}

			/** @brief  */
			virtual void			recruitWorker(const TString& typeName,const Worker* pWorker)
			{
				if( pWorker == NULL || typeName == TEXT("") )
					BLADE_EXCEPT(EXC_INVALID_PARAM, BTString("invalid registration.") );

				const Worker*& worker = mRegistry[typeName];
				if( worker != NULL )
					BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a worker with the same name \"")+typeName+BTString("\" already registered to the factory.") );

				mNameList.push_back(typeName);
				worker = pWorker;
			}

			/** @brief  */
			virtual const Worker*			findWorker(const TString& typeName, bool except = true) const
			{
				Registry::const_iterator i = mRegistry.find(typeName);
				if( i != mRegistry.end() )
					return i->second;
				else
				{
					if( except)
						BLADE_EXCEPT(EXC_NEXIST,BTString("a class with the name \"")+typeName+BTString("\" not registered to the factory.") );
					else
						return NULL;
				}
			}

			/** @brief  */
			virtual bool			removeWorker(const TString& typeName)
			{
				Registry::iterator i = mRegistry.find(typeName);
				if( i == mRegistry.end() )
				{
					assert(false);
					return false;
				}
				mRegistry.erase(i);
				WorkerList::iterator iter = std::find(mNameList.begin(), mNameList.end(), typeName);
				assert(iter != mNameList.end());
				mNameList.erase(iter);
				return true;
			}

			/** @brief  */
			virtual size_t			getWorkerCount() const
			{
				return mNameList.size();
			}

			/** @brief  */
			virtual const TString&	getWorkerType(index_t index) const
			{
				if( index < mNameList.size() )
				{
					WorkerList::const_iterator i = mNameList.begin();
					std::advance(i, index);
					return *i;
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range.") );
			}


		protected:
			typedef TStringMap< const Worker*, StaticAllocator<Worker*> > Registry;
			typedef StaticList<TString>	WorkerList;

			Registry		mRegistry;
			WorkerList		mNameList;
		};//class UniversalHR


		//////////////////////////////////////////////////////////////////////////
		HRD&		HRD::buildDepartment()
		{
			return *(BLADE_NEW UniversalHRD() );
		}

		//////////////////////////////////////////////////////////////////////////
		void		HRD::closeDepartment(HRD& hr)
		{
			//note: because HRD has no virtual dector
			//convert it to the right type and delete
			BLADE_DELETE (UniversalHRD*)&hr;
		}

	}//namespace FactoryUtil

}//namespace Blade