using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using System.Collections.Concurrent;

namespace JakeKnowsEngineComponent
{
    public partial class AssocApplication_NewsChannel : CTable
    {
        private SqlInt32 _IDApplication;
        private SqlInt32 _IDNewsChannel;

        #region functions
        protected override void SetID(SqlInt64 id) { _IDApplication = (SqlInt32)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_IDApplication; }
        public SqlInt64 GetAssociatedID() { return (SqlInt64)_IDNewsChannel; }
        public override void SetValue(SqlString val) { }

        public AssocApplication_NewsChannel() { }
        public AssocApplication_NewsChannel(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public AssocApplication_NewsChannel(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _IDApplication = rd.SafeGetInt32("IDApplication");
            _IDNewsChannel = rd.SafeGetInt32("IDNewsChannel");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDApplication", _IDApplication);
            cmd.Parameters.AddWithValue("@IDNewsChannel", _IDNewsChannel);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDApplication", _IDApplication);
            cmd.Parameters.AddWithValue("@IDNewsChannel", _IDNewsChannel);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDNewsChannel", _IDNewsChannel);
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDApplication);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@@colp@", _IDNewsChannel);
        }

        #endregion
        #region accessors
        public SqlInt32 IDApplication
        {
            get { return _IDApplication; }
            set
            {
                _IDApplication = value;
                IsDirty = true;
            }
        }
        public SqlInt32 IDNewsChannel
        {
            get { return _IDNewsChannel; }
            set
            {
                _IDNewsChannel = value;
                IsDirty = true;
            }
        }

        #endregion
    }
    public partial class LnkProfile_Group_Admin : CTable
    {
        private SqlInt64 _IDProfile;
        private SqlInt64 _IDGroup;

        #region functions
        protected override void SetID(SqlInt64 id) { _IDProfile = (SqlInt64)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_IDProfile; }
        public SqlInt64 GetAssociatedID() { return (SqlInt64)_IDGroup; }
        public override void SetValue(SqlString val) { }

        public LnkProfile_Group_Admin() { }
        public LnkProfile_Group_Admin(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public LnkProfile_Group_Admin(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _IDProfile = rd.SafeGetInt64("IDProfile");
            _IDGroup = rd.SafeGetInt64("IDGroup");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDProfile", _IDProfile);
            cmd.Parameters.AddWithValue("@IDGroup", _IDGroup);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDProfile", _IDProfile);
            cmd.Parameters.AddWithValue("@IDGroup", _IDGroup);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDGroup", _IDGroup);
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDProfile);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@@colp@", _IDGroup);
        }

        #endregion
        #region accessors
        public SqlInt64 IDProfile
        {
            get { return _IDProfile; }
            set
            {
                _IDProfile = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDGroup
        {
            get { return _IDGroup; }
            set
            {
                _IDGroup = value;
                IsDirty = true;
            }
        }

        #endregion
    }
    public partial class TblCountry : CTable
    {
        private SqlInt32 _IDCountry; //IDENTITY 
        private SqlString _Country;  //100
        private SqlString _FIPS;  //8
        private SqlString _ISO;  //4
        private SqlString _ISO3;  //6
        private SqlString _ISON;  //8
        private SqlString _Internet;  //4
        private SqlString _Capital;  //100
        private SqlString _MapReference;  //100
        private SqlString _NationalitySingular;  //80
        private SqlString _NationalityPlural;  //80
        private SqlString _Currency;  //80
        private SqlString _CurrencyCode;  //8
        private SqlString _Title;  //160
        public const int CountryMaxLen = 100;
        public const int FIPSMaxLen = 8;
        public const int ISOMaxLen = 4;
        public const int ISO3MaxLen = 6;
        public const int ISONMaxLen = 8;
        public const int InternetMaxLen = 4;
        public const int CapitalMaxLen = 100;
        public const int MapReferenceMaxLen = 100;
        public const int NationalitySingularMaxLen = 80;
        public const int NationalityPluralMaxLen = 80;
        public const int CurrencyMaxLen = 80;
        public const int CurrencyCodeMaxLen = 8;
        public const int TitleMaxLen = 160;

        #region functions
        protected override void SetID(SqlInt64 id) { _IDCountry = (SqlInt32)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_IDCountry; }
        public override void SetValue(SqlString val) { _Country = val; }

        public TblCountry() { }
        public TblCountry(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public TblCountry(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _IDCountry = rd.SafeGetInt32("IDCountry");
            _Country = rd.SafeGetString("Country");
            _FIPS = rd.SafeGetString("FIPS");
            _ISO = rd.SafeGetString("ISO");
            _ISO3 = rd.SafeGetString("ISO3");
            _ISON = rd.SafeGetString("ISON");
            _Internet = rd.SafeGetString("Internet");
            _Capital = rd.SafeGetString("Capital");
            _MapReference = rd.SafeGetString("MapReference");
            _NationalitySingular = rd.SafeGetString("NationalitySingular");
            _NationalityPlural = rd.SafeGetString("NationalityPlural");
            _Currency = rd.SafeGetString("Currency");
            _CurrencyCode = rd.SafeGetString("CurrencyCode");
            _Title = rd.SafeGetString("Title");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDCountry);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDCountry);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDCountry);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@Country", _Country);
            cmd.Parameters.AddWithValue("@FIPS", _FIPS);
            cmd.Parameters.AddWithValue("@ISO", _ISO);
            cmd.Parameters.AddWithValue("@ISO3", _ISO3);
            cmd.Parameters.AddWithValue("@ISON", _ISON);
            cmd.Parameters.AddWithValue("@Internet", _Internet);
            cmd.Parameters.AddWithValue("@Capital", _Capital);
            cmd.Parameters.AddWithValue("@MapReference", _MapReference);
            cmd.Parameters.AddWithValue("@NationalitySingular", _NationalitySingular);
            cmd.Parameters.AddWithValue("@NationalityPlural", _NationalityPlural);
            cmd.Parameters.AddWithValue("@Currency", _Currency);
            cmd.Parameters.AddWithValue("@CurrencyCode", _CurrencyCode);
            cmd.Parameters.AddWithValue("@Title", _Title);
        }

        #endregion
        #region accessors
        public SqlInt32 IDCountry
        {
            get { return _IDCountry; }
            private set
            {
                _IDCountry = value;
                IsDirty = true;
            }
        }
        public SqlString Country
        {
            get { return _Country; }
            set
            {
                _Country = value;
                IsDirty = true;
            }
        }
        public SqlString FIPS
        {
            get { return _FIPS; }
            set
            {
                _FIPS = value;
                IsDirty = true;
            }
        }
        public SqlString ISO
        {
            get { return _ISO; }
            set
            {
                _ISO = value;
                IsDirty = true;
            }
        }
        public SqlString ISO3
        {
            get { return _ISO3; }
            set
            {
                _ISO3 = value;
                IsDirty = true;
            }
        }
        public SqlString ISON
        {
            get { return _ISON; }
            set
            {
                _ISON = value;
                IsDirty = true;
            }
        }
        public SqlString Internet
        {
            get { return _Internet; }
            set
            {
                _Internet = value;
                IsDirty = true;
            }
        }
        public SqlString Capital
        {
            get { return _Capital; }
            set
            {
                _Capital = value;
                IsDirty = true;
            }
        }
        public SqlString MapReference
        {
            get { return _MapReference; }
            set
            {
                _MapReference = value;
                IsDirty = true;
            }
        }
        public SqlString NationalitySingular
        {
            get { return _NationalitySingular; }
            set
            {
                _NationalitySingular = value;
                IsDirty = true;
            }
        }
        public SqlString NationalityPlural
        {
            get { return _NationalityPlural; }
            set
            {
                _NationalityPlural = value;
                IsDirty = true;
            }
        }
        public SqlString Currency
        {
            get { return _Currency; }
            set
            {
                _Currency = value;
                IsDirty = true;
            }
        }
        public SqlString CurrencyCode
        {
            get { return _CurrencyCode; }
            set
            {
                _CurrencyCode = value;
                IsDirty = true;
            }
        }
        public SqlString Title
        {
            get { return _Title; }
            set
            {
                _Title = value;
                IsDirty = true;
            }
        }

        #endregion
    }
    public partial class TblLocationCity : CTable
    {
        private SqlInt64 _IDLocationCity; //IDENTITY 
        private SqlString _LocationCity;  //50
        public const int LocationCityMaxLen = 50;

        #region functions
        protected override void SetID(SqlInt64 id) { _IDLocationCity = (SqlInt64)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_IDLocationCity; }
        public override void SetValue(SqlString val) { _LocationCity = val; }

        public TblLocationCity() { }
        public TblLocationCity(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public TblLocationCity(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _IDLocationCity = rd.SafeGetInt64("IDLocationCity");
            _LocationCity = rd.SafeGetString("LocationCity");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDLocationCity);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDLocationCity);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDLocationCity);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@LocationCity", _LocationCity);
        }

        #endregion
        #region accessors
        public SqlInt64 IDLocationCity
        {
            get { return _IDLocationCity; }
            private set
            {
                _IDLocationCity = value;
                IsDirty = true;
            }
        }
        public SqlString LocationCity
        {
            get { return _LocationCity; }
            set
            {
                _LocationCity = value;
                IsDirty = true;
            }
        }

        #endregion
    }
    public partial class TblProfile : CTable
    {
        private SqlInt64 _IDProfile; //IDENTITY 
        private SqlInt64 _IDProfile_2;
        private SqlInt64 _IDMedia;
        private SqlInt64 _IDPhone_Primary;
        private SqlInt64 _IDPhone_Preferred;
        private SqlInt64 _IDEmail_Preferred;
        private SqlInt32 _TYDataAccess;
        private SqlString _Password;  //20
        private SqlBoolean _UseCommSMS;
        private SqlBoolean _UseCommEmail;
        private SqlBoolean _UseCommIVR;
        private SqlBoolean _IsConfirmed;
        private SqlByte _AdminNotification;
        private SqlInt64 _DataManagementTypeID;
        private SqlInt64 _ArchiveID;
        private SqlByte _QueueDigest;
        private SqlDateTime _Updated;
        private SqlDateTime _LastNotification;
        private SqlDateTime _LastUpdateSAQ;
        private SqlDecimal _SAQ;
        public const int PasswordMaxLen = 20;
        private AssocProfile_ApplicationAssoc _AssocProfile_Application;
        private AssocProfile_ArchiveRecordAssoc _AssocProfile_ArchiveRecord;
        private AssocProfile_CompanyAssoc _AssocProfile_Company;
        private AssocProfile_ContactAssoc _AssocProfile_Contact;
        private AssocProfile_Contact_InterestAttributesAssoc _AssocProfile_Contact_InterestAttributes;
        private AssocProfile_EmailAssoc _AssocProfile_Email;
        private AssocProfile_GroupAssoc _AssocProfile_Group;
        private AssocProfile_InterestAttributeAssoc _AssocProfile_InterestAttribute;
        private AssocProfile_LocationAssoc _AssocProfile_Location;
        private AssocProfile_MediaAssoc _AssocProfile_Media;
        private AssocProfile_Media_RelationshipAssoc _AssocProfile_Media_Relationship;
        private AssocProfile_MerchantAssoc _AssocProfile_Merchant;
        private AssocProfile_MetroAreaAssoc _AssocProfile_MetroArea;
        private AssocProfile_NoteAssoc _AssocProfile_Note;
        private AssocProfile_PersonAssoc _AssocProfile_Person;
        private AssocProfile_PhoneAssoc _AssocProfile_Phone;
        private AssocProfile_PrefixAssoc _AssocProfile_Prefix;
        private AssocProfile_ProfileAssoc _AssocProfile_Profile;
        private AssocProfile_TitleAssoc _AssocProfile_Title;
        private AssocProfile_WebAddressAssoc _AssocProfile_WebAddress;
        private LnkProfile_ContactAssoc _LnkProfile_Contact;
        private LnkProfile_DataSource_PrioritySearchAssoc _LnkProfile_DataSource_PrioritySearch;
        private LnkProfile_Group_AdminAssoc _LnkProfile_Group_Admin;
        private LnkProfile_Media_HashAssoc _LnkProfile_Media_Hash;
        private LnkProfile_Profile_MergeReviewAssoc _LnkProfile_Profile_MergeReview;
        private OvrProfile_DataAccessAssoc _OvrProfile_DataAccess;
        private OvrProfile_DataManagementAssoc _OvrProfile_DataManagement;
        private OvrProfile_Profile_RelationshipTypeAssoc _OvrProfile_Profile_RelationshipType;
        private ReviewProfile_ApplicationAssoc _ReviewProfile_Application;
        private ReviewProfile_ProfileAssoc _ReviewProfile_Profile;
        ConcurrentDictionary<DataObjects.TableTypes, CTableAssociation> _associations = new ConcurrentDictionary<DataObjects.TableTypes, CTableAssociation>();
        public override ConcurrentDictionary<DataObjects.TableTypes, CTableAssociation> GetAssociations() { return _associations; }

        #region functions
        protected override void SetID(SqlInt64 id) { _IDProfile = (SqlInt64)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_IDProfile; }
        public override void SetValue(SqlString val) { }

        public TblProfile() { }
        public TblProfile(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public TblProfile(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
            _AssocProfile_Application = new AssocProfile_ApplicationAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Application, _AssocProfile_Application);
            _AssocProfile_ArchiveRecord = new AssocProfile_ArchiveRecordAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_ArchiveRecord, _AssocProfile_ArchiveRecord);
            _AssocProfile_Company = new AssocProfile_CompanyAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Company, _AssocProfile_Company);
            _AssocProfile_Contact = new AssocProfile_ContactAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Contact, _AssocProfile_Contact);
            _AssocProfile_Contact_InterestAttributes = new AssocProfile_Contact_InterestAttributesAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Contact_InterestAttributes, _AssocProfile_Contact_InterestAttributes);
            _AssocProfile_Email = new AssocProfile_EmailAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Email, _AssocProfile_Email);
            _AssocProfile_Group = new AssocProfile_GroupAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Group, _AssocProfile_Group);
            _AssocProfile_InterestAttribute = new AssocProfile_InterestAttributeAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_InterestAttribute, _AssocProfile_InterestAttribute);
            _AssocProfile_Location = new AssocProfile_LocationAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Location, _AssocProfile_Location);
            _AssocProfile_Media = new AssocProfile_MediaAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Media, _AssocProfile_Media);
            _AssocProfile_Media_Relationship = new AssocProfile_Media_RelationshipAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Media_Relationship, _AssocProfile_Media_Relationship);
            _AssocProfile_Merchant = new AssocProfile_MerchantAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Merchant, _AssocProfile_Merchant);
            _AssocProfile_MetroArea = new AssocProfile_MetroAreaAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_MetroArea, _AssocProfile_MetroArea);
            _AssocProfile_Note = new AssocProfile_NoteAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Note, _AssocProfile_Note);
            _AssocProfile_Person = new AssocProfile_PersonAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Person, _AssocProfile_Person);
            _AssocProfile_Phone = new AssocProfile_PhoneAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Phone, _AssocProfile_Phone);
            _AssocProfile_Prefix = new AssocProfile_PrefixAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Prefix, _AssocProfile_Prefix);
            _AssocProfile_Profile = new AssocProfile_ProfileAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Profile, _AssocProfile_Profile);
            _AssocProfile_Title = new AssocProfile_TitleAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_Title, _AssocProfile_Title);
            _AssocProfile_WebAddress = new AssocProfile_WebAddressAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TAssocProfile_WebAddress, _AssocProfile_WebAddress);
            _LnkProfile_Contact = new LnkProfile_ContactAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TLnkProfile_Contact, _LnkProfile_Contact);
            _LnkProfile_DataSource_PrioritySearch = new LnkProfile_DataSource_PrioritySearchAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TLnkProfile_DataSource_PrioritySearch, _LnkProfile_DataSource_PrioritySearch);
            _LnkProfile_Group_Admin = new LnkProfile_Group_AdminAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TLnkProfile_Group_Admin, _LnkProfile_Group_Admin);
            _LnkProfile_Media_Hash = new LnkProfile_Media_HashAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TLnkProfile_Media_Hash, _LnkProfile_Media_Hash);
            _LnkProfile_Profile_MergeReview = new LnkProfile_Profile_MergeReviewAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TLnkProfile_Profile_MergeReview, _LnkProfile_Profile_MergeReview);
            _OvrProfile_DataAccess = new OvrProfile_DataAccessAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TOvrProfile_DataAccess, _OvrProfile_DataAccess);
            _OvrProfile_DataManagement = new OvrProfile_DataManagementAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TOvrProfile_DataManagement, _OvrProfile_DataManagement);
            _OvrProfile_Profile_RelationshipType = new OvrProfile_Profile_RelationshipTypeAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TOvrProfile_Profile_RelationshipType, _OvrProfile_Profile_RelationshipType);
            _ReviewProfile_Application = new ReviewProfile_ApplicationAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TReviewProfile_Application, _ReviewProfile_Application);
            _ReviewProfile_Profile = new ReviewProfile_ProfileAssoc(GetID());
            _associations.TryAdd(DataObjects.TableTypes.TReviewProfile_Profile, _ReviewProfile_Profile);
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _IDProfile = rd.SafeGetInt64("IDProfile");
            _IDProfile_2 = rd.SafeGetInt64("IDProfile_2");
            _IDMedia = rd.SafeGetInt64("IDMedia");
            _IDPhone_Primary = rd.SafeGetInt64("IDPhone_Primary");
            _IDPhone_Preferred = rd.SafeGetInt64("IDPhone_Preferred");
            _IDEmail_Preferred = rd.SafeGetInt64("IDEmail_Preferred");
            _TYDataAccess = rd.SafeGetInt32("TYDataAccess");
            _Password = rd.SafeGetString("Password");
            _UseCommSMS = rd.SafeGetBoolean("UseCommSMS");
            _UseCommEmail = rd.SafeGetBoolean("UseCommEmail");
            _UseCommIVR = rd.SafeGetBoolean("UseCommIVR");
            _IsConfirmed = rd.SafeGetBoolean("IsConfirmed");
            _AdminNotification = rd.SafeGetByte("AdminNotification");
            _DataManagementTypeID = rd.SafeGetInt64("DataManagementTypeID");
            _ArchiveID = rd.SafeGetInt64("ArchiveID");
            _QueueDigest = rd.SafeGetByte("QueueDigest");
            _Updated = rd.SafeGetDateTime("Updated");
            _LastNotification = rd.SafeGetDateTime("LastNotification");
            _LastUpdateSAQ = rd.SafeGetDateTime("LastUpdateSAQ");
            _SAQ = rd.SafeGetDecimal("SAQ");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDProfile);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDProfile);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _IDProfile);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@IDProfile_2", _IDProfile_2);
            cmd.Parameters.AddWithValue("@IDMedia", _IDMedia);
            cmd.Parameters.AddWithValue("@IDPhone_Primary", _IDPhone_Primary);
            cmd.Parameters.AddWithValue("@IDPhone_Preferred", _IDPhone_Preferred);
            cmd.Parameters.AddWithValue("@IDEmail_Preferred", _IDEmail_Preferred);
            cmd.Parameters.AddWithValue("@TYDataAccess", _TYDataAccess);
            cmd.Parameters.AddWithValue("@Password", _Password);
            cmd.Parameters.AddWithValue("@UseCommSMS", _UseCommSMS);
            cmd.Parameters.AddWithValue("@UseCommEmail", _UseCommEmail);
            cmd.Parameters.AddWithValue("@UseCommIVR", _UseCommIVR);
            cmd.Parameters.AddWithValue("@IsConfirmed", _IsConfirmed);
            cmd.Parameters.AddWithValue("@AdminNotification", _AdminNotification);
            cmd.Parameters.AddWithValue("@DataManagementTypeID", _DataManagementTypeID);
            cmd.Parameters.AddWithValue("@ArchiveID", _ArchiveID);
            cmd.Parameters.AddWithValue("@QueueDigest", _QueueDigest);
            cmd.Parameters.AddWithValue("@Updated", _Updated);
            cmd.Parameters.AddWithValue("@LastNotification", _LastNotification);
            cmd.Parameters.AddWithValue("@LastUpdateSAQ", _LastUpdateSAQ);
            cmd.Parameters.AddWithValue("@SAQ", _SAQ);
        }

        #endregion
        #region accessors
        public SqlInt64 IDProfile
        {
            get { return _IDProfile; }
            private set
            {
                _IDProfile = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDProfile_2
        {
            get { return _IDProfile_2; }
            set
            {
                _IDProfile_2 = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDMedia
        {
            get { return _IDMedia; }
            set
            {
                _IDMedia = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDPhone_Primary
        {
            get { return _IDPhone_Primary; }
            set
            {
                _IDPhone_Primary = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDPhone_Preferred
        {
            get { return _IDPhone_Preferred; }
            set
            {
                _IDPhone_Preferred = value;
                IsDirty = true;
            }
        }
        public SqlInt64 IDEmail_Preferred
        {
            get { return _IDEmail_Preferred; }
            set
            {
                _IDEmail_Preferred = value;
                IsDirty = true;
            }
        }
        public SqlInt32 TYDataAccess
        {
            get { return _TYDataAccess; }
            set
            {
                _TYDataAccess = value;
                IsDirty = true;
            }
        }
        public SqlString Password
        {
            get { return _Password; }
            set
            {
                _Password = value;
                IsDirty = true;
            }
        }
        public SqlBoolean UseCommSMS
        {
            get { return _UseCommSMS; }
            set
            {
                _UseCommSMS = value;
                IsDirty = true;
            }
        }
        public SqlBoolean UseCommEmail
        {
            get { return _UseCommEmail; }
            set
            {
                _UseCommEmail = value;
                IsDirty = true;
            }
        }
        public SqlBoolean UseCommIVR
        {
            get { return _UseCommIVR; }
            set
            {
                _UseCommIVR = value;
                IsDirty = true;
            }
        }
        public SqlBoolean IsConfirmed
        {
            get { return _IsConfirmed; }
            set
            {
                _IsConfirmed = value;
                IsDirty = true;
            }
        }
        public SqlByte AdminNotification
        {
            get { return _AdminNotification; }
            set
            {
                _AdminNotification = value;
                IsDirty = true;
            }
        }
        public SqlInt64 DataManagementTypeID
        {
            get { return _DataManagementTypeID; }
            set
            {
                _DataManagementTypeID = value;
                IsDirty = true;
            }
        }
        public SqlInt64 ArchiveID
        {
            get { return _ArchiveID; }
            set
            {
                _ArchiveID = value;
                IsDirty = true;
            }
        }
        public SqlByte QueueDigest
        {
            get { return _QueueDigest; }
            set
            {
                _QueueDigest = value;
                IsDirty = true;
            }
        }
        public SqlDateTime Updated
        {
            get { return _Updated; }
            set
            {
                _Updated = value;
                IsDirty = true;
            }
        }
        public SqlDateTime LastNotification
        {
            get { return _LastNotification; }
            set
            {
                _LastNotification = value;
                IsDirty = true;
            }
        }
        public SqlDateTime LastUpdateSAQ
        {
            get { return _LastUpdateSAQ; }
            set
            {
                _LastUpdateSAQ = value;
                IsDirty = true;
            }
        }
        public SqlDecimal SAQ
        {
            get { return _SAQ; }
            set
            {
                _SAQ = value;
                IsDirty = true;
            }
        }

        #endregion
    }
    public partial class TypeZipCodeLocation : CTable
    {
        private SqlInt32 _TYZipCodeLocation; //IDENTITY 
        private SqlString _ZipCodeLocation;  //15
        public const int ZipCodeLocationMaxLen = 15;

        #region functions
        protected override void SetID(SqlInt64 id) { _TYZipCodeLocation = (SqlInt32)id; }
        public override SqlInt64 GetID() { return (SqlInt64)_TYZipCodeLocation; }
        public override void SetValue(SqlString val) { _ZipCodeLocation = val; }

        public TypeZipCodeLocation() { }
        public TypeZipCodeLocation(SqlInt64 id, SqlConnection cn, SqlCommand cmd, SqlDataReader rd) //initialize from its ID
        {
            SetID(id);
            OnPreRetrieve(cmd);
            Retrieve(cn, cmd, rd);
            Init();
        }
        public TypeZipCodeLocation(SqlDataReader rd)
        {
            InitFromData(rd);
            Init();
        }

        private void Init()
        {
            IsDirty = false;
            IsValid = false;
        }
        protected override Result InitFromData(SqlDataReader rd)
        {
            _TYZipCodeLocation = rd.SafeGetInt32("TYZipCodeLocation");
            _ZipCodeLocation = rd.SafeGetString("ZipCodeLocation");
            IsDirty = false;
            IsValid = true;
            return Result.OK;
        }
        protected override Result OnPreRetrieve(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _TYZipCodeLocation);
            return Result.OK;
        }
        protected override Result OnPreDelete(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _TYZipCodeLocation);
            return Result.OK;
        }
        protected override Result OnPreCreate(SqlCommand cmd)
        {
            AddParameters(cmd);
            return Result.OK;
        }
        protected override Result OnPreUpdate(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@RecordID", _TYZipCodeLocation);
            AddParameters(cmd);
            return Result.OK;
        }
        private void AddParameters(SqlCommand cmd)
        {
            cmd.Parameters.AddWithValue("@ZipCodeLocation", _ZipCodeLocation);
        }

        #endregion
        #region accessors
        public SqlInt32 TYZipCodeLocation
        {
            get { return _TYZipCodeLocation; }
            private set
            {
                _TYZipCodeLocation = value;
                IsDirty = true;
            }
        }
        public SqlString ZipCodeLocation
        {
            get { return _ZipCodeLocation; }
            set
            {
                _ZipCodeLocation = value;
                IsDirty = true;
            }
        }

        #endregion
    }
}