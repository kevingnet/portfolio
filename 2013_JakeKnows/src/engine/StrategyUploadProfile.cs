using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using System.Data.SqlTypes;
using log4net;

namespace JakeKnowsEngineComponent
{
	public class StrategyUploadProfile : CStrategyOperation //uploadProfile
	{
		public override bool Execute(WSCall ws, DataObjects obj)
		{
			SqlConnection cn = null;
			Logging.LogDebug("Execute");
            try
            {
				int version = ws.GetVersion();
				string Device = ws.GetParameterAs_string("deviceUID"); //Device 
				string Token = ws.GetParameterAs_string("deviceToken"); //Token 
				string PhoneNumber = ws.GetParameterAs_string("devicePhoneNumber"); //PhoneNumber 
				string FirstName = ws.GetParameterAs_string("contactFirstName"); //FirstName 
				...
				string XMLNote = ws.GetParameterAs_string("noteXML"); //XMLNote 
				bool CompressInput = ws.GetParameterAs_bool("compressionFlagInput"); //CompressInput 

				cn = new SqlConnection(JakeKnowsEngine.ConnectionString);
				cn.Open();
				
				SqlInt64 IDDevice = obj.GetIDFrom(cn, ColumnTypes.TIDDevice, Device);
				SqlInt64 IDPerson_FirstName = obj.GetIDFrom(cn, ColumnTypes.TIDPerson, FirstName);
				...
				SqlInt64 IDPhone_PhonePager = obj.GetIDFrom(cn, ColumnTypes.TIDPhone, PhonePager);
				SqlInt64 IDPhone_PhoneOther = obj.GetIDFrom(cn, ColumnTypes.TIDPhone, PhoneOther);

				cn.Close();
				cn.Dispose();
            }
            catch (Exception e)
            {
                log.Error("Exception " + e.Message);
            }
			finally
			{
				if (cn != null)
				{
					if (cn.State != System.Data.ConnectionState.Closed)
						cn.Close();
					cn.Dispose();
				}
			}
            return true;
		}
	}
}
