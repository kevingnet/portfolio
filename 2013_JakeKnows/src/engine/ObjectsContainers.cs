using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Collections.Concurrent;
using System.Data.SqlTypes;
using System.Configuration;
using System.Diagnostics;
using log4net;
using System.Threading;
using System.Threading.Tasks;
using System.Reflection;

namespace JakeKnowsEngineComponent
{
    public partial class DataObjects
    {
        public enum TableTypes
        {
            TTABLEINVALID,
            TAssocApplication_NewsChannel,
            TAssocContact_Company,
            ...
            TTypeZipCode,
            TTypeZipCodeLocation
        }

        protected static readonly ILog log = LogManager.GetLogger(System.Reflection.MethodBase.GetCurrentMethod().DeclaringType);
        FieldsDictionary m_Fields = null;
        ConcurrentDictionary<TableTypes, CData> _tables = new ConcurrentDictionary<TableTypes, CData>();
        private FltDevice_DataInputData _FltDevice_DataInput = new FltDevice_DataInputData(0, 0);
        private FltProfile_DataAccessData _FltProfile_DataAccess = new FltProfile_DataAccessData(0, 0);
        ...
        private TypeTitleData _TypeTitle = new TypeTitleData(0, 0);
        private TypeZipCodeData _TypeZipCode = new TypeZipCodeData(0, 0);


        public static TableTypes GetTableType(string table)
        {
            TableTypes type = TableTypes.TTABLEINVALID;
            if (Enum.IsDefined(typeof(TableTypes), table))
                type = (TableTypes)Enum.Parse(typeof(TableTypes), table, true);
            return type;
        }

        public void Init(FieldsDictionary fields)
        {
            m_Fields = fields;
            _tables.TryAdd(TableTypes.TFltDevice_DataInput, _FltDevice_DataInput);
            _tables.TryAdd(TableTypes.TFltProfile_DataAccess, _FltProfile_DataAccess);
            ...
            _tables.TryAdd(TableTypes.TTypeTitle, _TypeTitle);
            _tables.TryAdd(TableTypes.TTypeZipCode, _TypeZipCode);

            LoadData();
        }
    }
}
