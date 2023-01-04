using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Data.SqlClient;
using log4net;

namespace JakeKnowsEngineComponent
{
	public class ContextStrategy
	{
		protected static Logging debug = new Logging();
		protected static readonly ILog log = LogManager.GetLogger(System.Reflection.MethodBase.GetCurrentMethod().DeclaringType);
        private static Dictionary<int, IStrategyOperation> _strategies = new Dictionary<int, IStrategyOperation>();
		
		public static void Init()
		{
			_strategies.Add((int)StrategyOperation.WS_CheckVersion, new StrategyCheckVersion());
			_strategies.Add((int)StrategyOperation.WS_RegisterDeviceAndSubscriber, new StrategyRegisterDeviceAndSubscriber());
			_strategies.Add((int)StrategyOperation.WS_VerifyProfileByEmail, new StrategyVerifyProfileByEmail());
			_strategies.Add((int)StrategyOperation.WS_RegisterSubscriberByServer, new StrategyRegisterSubscriberByServer());
			_strategies.Add((int)StrategyOperation.WS_GetProfileByServer, new StrategyGetProfileByServer());
			_strategies.Add((int)StrategyOperation.WS_AssociateContactToGroup, new StrategyAssociateContactToGroup());
			_strategies.Add((int)StrategyOperation.WS_BatchGetMyPhonesProfiles, new StrategyBatchGetMyPhonesProfiles());
			_strategies.Add((int)StrategyOperation.WS_BatchUpdatePIM, new StrategyBatchUpdatePIM());
			_strategies.Add((int)StrategyOperation.WS_ConfirmDeviceNotification, new StrategyConfirmDeviceNotification());
			...
			_strategies.Add((int)StrategyOperation.WS_GetNews, new StrategyGetNews());

		}

        public static bool Execute(WSCall ws, DataObjects obj)
		{
            int op = ws.GetFunctionID();
            Logging.LogDebug("Execute op " + op);
            if (op == (int)StrategyOperation.WS_INVALID)
			{
                log.Error("Execute op WS_INVALID");
                return false;
			}
            return _strategies[op].Execute(ws, obj);
		}	
	public enum StrategyOperation
	{
		WS_INVALID,
		WS_CheckVersion, //checkWebServiceVersion
		WS_RegisterDeviceAndSubscriber, //registerDeviceAndSubscriber
		WS_RegisterDeviceAndSubscriberExtraData, //registerDeviceAndSubscriberWithEnhancedData
		...
		WS_GetNews, //getNews
	}
	}
}
