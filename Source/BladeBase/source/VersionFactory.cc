/********************************************************************
	created:	2013/04/01
	filename: 	VersionFactory.cc
	author:		Crazii
	purpose:	
*********************************************************************/
#include <BladePCH.h>
#include <VersionFactory.h>

namespace Blade
{
	namespace FactoryUtil
	{
		class UniversalVTHRD : public VTHRD , public StaticAllocatable
		{
		public:
			UniversalVTHRD()	{}
			virtual ~UniversalVTHRD()	{}

			/** @brief  */
			virtual void			recruitWorker(const TString& TypeName, Version version, const Worker* pWorker)
			{
				if( pWorker == NULL || TypeName == TString::EMPTY )
				{
					assert(false);
					BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid registration.") );
				}
				const Worker*& worker = mWorkerList[TypeName][version];
				if( worker != NULL )
				{
					assert(false);
					BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a worker with the same name \"")+TypeName+BTString("\" already registered to the factory.") );
				}
				worker = pWorker;
			}

			/** @brief  */
			virtual const Worker*	findWorker(const TString& TypeName, Version version, bool except = true) const
			{
				WorkerList::const_iterator i = mWorkerList.find(TypeName);
				if( i == mWorkerList.end() )
				{
					if( except )
						BLADE_EXCEPT(EXC_NEXIST,BTString("a class with the name \"")+TypeName+BTString("\" not registered to the factory.") );
					else
						return NULL;
				}

				const VersionWorkerMap& versionMap = i->second;
				VersionWorkerMap::const_iterator n = versionMap.find( version );
				if( n == versionMap.end() )
				{
					if( except )
						BLADE_EXCEPT(EXC_NEXIST,BTString("a class with the version \"")+ version.getVersionString() +BTString("\" not registered to the factory.") );
					else
						return NULL;
				}
				else
					return n->second;
			}

			/** @brief type count */
			virtual size_t			getWorkerCount() const
			{
				return mWorkerList.size();
			}

			/** @brief  */
			virtual const TString&	getWorkerType(index_t typeIndex) const
			{
				if( typeIndex < mWorkerList.size() )
				{
					WorkerList::const_iterator i = mWorkerList.begin();
					while( typeIndex-- > 0 )
						++i;
					return i->first;
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range") );
			}

			/** @brief version count for same type */
			virtual size_t			getVersionCount(index_t typeIndex) const
			{
				if( typeIndex < mWorkerList.size() )
				{
					WorkerList::const_iterator i = mWorkerList.begin();
					while( typeIndex-- > 0 )
						++i;
					return i->second.size();
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range") );
			}

			/** @brief  */
			virtual Version			getVersion(index_t typeIndex, index_t versionIndex) const
			{
				if( typeIndex < mWorkerList.size() )
				{
					WorkerList::const_iterator i = mWorkerList.begin();
					while( typeIndex-- > 0 )
						++i;
					const VersionWorkerMap& versionMap = i->second;
					VersionWorkerMap::const_iterator n = versionMap.begin();

					if( versionIndex < versionMap.size() )
					{
						while( versionIndex-- > 0 )
							++n;
						return n->first;
					}
					else
						BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range") );
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range") );
			}

			typedef Map<Version,const Worker*>	VersionWorkerMap;
			typedef TStringMap<VersionWorkerMap>	WorkerList;

			WorkerList		mWorkerList;
		};
		//////////////////////////////////////////////////////////////////////////
		VTHRD&			VTHRD::buildDepartment()
		{
			return *(BLADE_NEW UniversalVTHRD());
		}
		//////////////////////////////////////////////////////////////////////////
		void				VTHRD::closeDepartment(VTHRD& pd)
		{
			//note: because VTHRD has no virtual dector
			//convert it to the right type and delete

			UniversalVTHRD* ptr = (UniversalVTHRD*)&pd;
			BLADE_DELETE ptr;
		}



		/************************************************************************/
		/*                                                                      */
		/************************************************************************/
		class UniversalVHRD : public VHRD , public StaticAllocatable
		{
		public:
			UniversalVHRD()		{}
			virtual ~UniversalVHRD()	{}

			/** @brief  */
			virtual void			recruitWorker(Version version, const Worker* pWorker)
			{
				if( pWorker == NULL)
				{
					assert(false);
					BLADE_EXCEPT(EXC_INVALID_PARAM,BTString("invalid registration.") );
				}
				const Worker*& worker = mWorkerList[version];
				if( worker != NULL )
				{
					assert(false);
					BLADE_EXCEPT(EXC_REDUPLICATE,BTString("a worker with the same verstion \"")+
						version.getVersionString() +BTString("\" already registered to the factory.") );
				}
				worker = pWorker;
			}

			/** @brief  */
			virtual const Worker*	findWorker(Version version, bool except = true) const
			{
				WorkerList::const_iterator i = mWorkerList.find(version);
				if( i == mWorkerList.end() )
				{
					if( except )
						BLADE_EXCEPT(EXC_NEXIST,BTString("a class with the version \"")+
						version.getVersionString() +BTString("\" not registered to the factory.") );
					else
						return NULL;
				}
				else
					return i->second;
			}

			/** @brief version count for same type */
			virtual size_t			getVersionCount(index_t /*typeIndex*/) const
			{
				return mWorkerList.size();
			}

			/** @brief  */
			virtual Version			getVersion(index_t versionIndex) const
			{
				if( versionIndex < mWorkerList.size() )
				{
					WorkerList::const_iterator n = mWorkerList.begin();
					while( versionIndex-- > 0 )
						++n;
					return n->first;
				}
				else
					BLADE_EXCEPT(EXC_OUT_OF_RANGE, BTString("index out of range") );
			}

			typedef Map<Version,const Worker*>	WorkerList;
			WorkerList		mWorkerList;
		};

		//////////////////////////////////////////////////////////////////////////
		VHRD&			VHRD::buildDepartment()
		{
			return *(BLADE_NEW UniversalVHRD());
		}

		//////////////////////////////////////////////////////////////////////////
		void			VHRD::closeDepartment(VHRD& pd)
		{
			//note: because VHRD has no virtual dector
			//convert it to the right type and delete
			UniversalVHRD* ptr = (UniversalVHRD*)&pd;
			BLADE_DELETE ptr;
		}
	}
}//namespace Blade