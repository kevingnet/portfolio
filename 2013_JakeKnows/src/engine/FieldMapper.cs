using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Collections.Concurrent;
using System.Data.SqlTypes;

namespace JakeKnowsEngineComponent
{
    public enum FieldTypes
    {
        FieldType_Plain,
        FieldType_ID,
        FieldType_Type,
        FieldType_Status
    }

    public enum Input_Types
    {
        IN_string,
        IN_int,
        IN_decimal,
        IN_DateTime,
        IN_bool,
        IN_float
    }

    public enum SQL_Types
    {
        SQL_bigint,
        SQL_bit,
        SQL_datetime,
        SQL_decimal,
        SQL_float,
        SQL_image,
        SQL_int,
        SQL_money,
        SQL_nvarchar,
        SQL_smallint,
        SQL_tinyint,
        SQL_varchar
    }
    public class InputFieldInfo
    {
        public string Name { get; private set; }
        public bool IsIdentity { get; private set; }
        public bool IsSecure { get; private set; }
        public bool IsRequired { get; private set; }
        public Input_Types Type { get; private set; }
        public string ConvertTo { get; private set; }
        public string ValidateTo { get; private set; }
        public string ResolvesTo { get; private set; }

        private InputFieldInfo() { }
        public InputFieldInfo(string name, bool id, bool secure, bool required, Input_Types inputType, string sqlType, string validatesTo, string resolvesTo)
        {
            Name = name;
            IsIdentity = id;
            IsSecure = secure;
            IsRequired = required;
            Type = inputType;
            ConvertTo = sqlType;
            ValidateTo = validatesTo;
            ResolvesTo = resolvesTo;
        }
    }

    public enum ColumnTypes
    {
        TCOLUMNINVALID,
        TIDAccessKey,
        TIDApplication,
        TIDAreaCode,
        TIDCity,
        ...
        TTYZipCode,
        TTYZipCodeLocation,
    }

    public class FieldsDictionary
    {
        static int initialCapacity = 101;
        int numProcs = Environment.ProcessorCount;
        static int concurrencyLevel = Environment.ProcessorCount * 2;

        ConcurrentDictionary<ColumnTypes, DataObjects.TableTypes> _idFieldTableMap = new ConcurrentDictionary<ColumnTypes, DataObjects.TableTypes>();
        ConcurrentDictionary<string, InputFieldInfo> _InputFieldMapper = new ConcurrentDictionary<string, InputFieldInfo>(concurrencyLevel, initialCapacity);

        public DataObjects.TableTypes GetTableType(ColumnTypes field)
        {
            return _idFieldTableMap[field];
        }

        public string GetNewName(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.Name;
        }
        public string GetValidateTo(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.ValidateTo;
        }
        public string GetResolveTo(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.ResolvesTo;
        }
        public bool IsIdentity(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.IsIdentity;
        }
        public bool IsSecure(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.IsSecure;
        }
        public bool IsRequred(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.IsRequired;
        }
        public Input_Types GetType(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.Type;
        }
        public string GetConvertTo(string param)
        {
            InputFieldInfo fi = _InputFieldMapper[param];
            return fi.ConvertTo;
        }

        public static bool IsID(string col)
        {
            return col.Substring(0, 2) == "ID";
        }

        public static bool IsType(string col)
        {
            return col.Substring(0, 2) == "TY";
        }

        public static bool IsStatus(string col)
        {
            return col.Substring(0, 2) == "ST";
        }

        public static FieldTypes GetFieldType(string type)
        {
            switch (type.Substring(0, 2))
            {
                case "ID":
                    return FieldTypes.FieldType_ID;
                case "TY":
                    return FieldTypes.FieldType_Type;
                case "ST":
                    return FieldTypes.FieldType_Status;
                default:
                    return FieldTypes.FieldType_Plain;
            }
        }

        public static SQL_Types GetSQL_Type(string type)
        {
            switch (type)
            {
                case "bigint":
                    return SQL_Types.SQL_bigint;
                case "bit":
                    return SQL_Types.SQL_bit;
                case "datetime":
                    return SQL_Types.SQL_datetime;
                case "decimal":
                    return SQL_Types.SQL_decimal;
                case "float":
                    return SQL_Types.SQL_float;
                case "image":
                    return SQL_Types.SQL_image;
                case "int":
                    return SQL_Types.SQL_int;
                case "money":
                    return SQL_Types.SQL_money;
                case "nvarchar":
                    return SQL_Types.SQL_nvarchar;
                case "smallint":
                    return SQL_Types.SQL_smallint;
                case "tinyint":
                    return SQL_Types.SQL_tinyint;
                case "varchar":
                    return SQL_Types.SQL_varchar;
                default:
                    return SQL_Types.SQL_tinyint;
            }
        }

        public static Input_Types GetInput_Type(string type)
        {
            switch (type)
            {
                case "string":
                    return Input_Types.IN_string;
                case "int":
                    return Input_Types.IN_int;
                case "DateTime":
                    return Input_Types.IN_DateTime;
                case "decimal":
                    return Input_Types.IN_decimal;
                case "double":
                    return Input_Types.IN_float;
                case "bool":
                    return Input_Types.IN_bool;
                default:
                    return Input_Types.IN_string;
            }
        }

        public void Init()
        {
            _idFieldTableMap[ColumnTypes.TIDAccessKey] = DataObjects.TableTypes.TTblAccessKey;
            _idFieldTableMap[ColumnTypes.TIDApplication] = DataObjects.TableTypes.TTblApplication;
            _idFieldTableMap[ColumnTypes.TIDAreaCode] = DataObjects.TableTypes.TTblAreaCodeRegion;
            _idFieldTableMap[ColumnTypes.TIDCity] = DataObjects.TableTypes.TTblCity;
            _idFieldTableMap[ColumnTypes.TIDCompany] = DataObjects.TableTypes.TTblCompany;
            ...
            _idFieldTableMap[ColumnTypes.TTYZipCode] = DataObjects.TableTypes.TTypeZipCode;
            _idFieldTableMap[ColumnTypes.TTYZipCodeLocation] = DataObjects.TableTypes.TTypeZipCodeLocation;

            _InputFieldMapper[""] = new InputFieldInfo("", false, false, true, GetInput_Type(""), "", "", "");
            _InputFieldMapper["accessKey"] = new InputFieldInfo("AccessKey", false, true, true, GetInput_Type("string"), "String", "GUID", "AccessKey");
            _InputFieldMapper["AccountSid"] = new InputFieldInfo("SID", false, false, false, GetInput_Type("string"), "String", "string", "");
            _InputFieldMapper["allMembersFlag"] = new InputFieldInfo("UseAllMembers", false, false, true, GetInput_Type("int"), "Boolean", "", "");
            _InputFieldMapper["annotation"] = new InputFieldInfo("Annotation", false, false, true, GetInput_Type("string"), "String", "string", "");
            _InputFieldMapper["appConB64String"] = new InputFieldInfo("ApplicationData", false, false, true, GetInput_Type("string"), "String", "", "");
            ...
            _InputFieldMapper["workPostalCode"] = new InputFieldInfo("ZipCodeWork", false, false, true, GetInput_Type("string"), "String", "zip", "IDLocationZipCode");
            _InputFieldMapper["workState"] = new InputFieldInfo("StateWork", false, false, true, GetInput_Type("string"), "String", "state", "IDLocationState");
            _InputFieldMapper["year"] = new InputFieldInfo("ExpirationYear", false, false, true, GetInput_Type("string"), "Int16", "int", "ExpirationYear");

        }
    }
}
