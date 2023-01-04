using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Threading;
using System.ServiceModel;
using System.ServiceModel.Description;
using System.Text.RegularExpressions;
using System.Data.SqlClient;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using System.Runtime.Remoting.Messaging;
using log4net;
using System.Xml;
using System.Xml.Linq;
using System.Configuration;

[assembly: CLSCompliant(true)]
namespace JakeKnowsEngineComponent
{
    //[ServiceBehavior(InstanceContextMode = InstanceContextMode.Single, ConcurrencyMode = ConcurrencyMode.Single, UseSynchronizationContext = false)]
    [ServiceBehavior(InstanceContextMode = InstanceContextMode.PerCall, ConcurrencyMode = ConcurrencyMode.Multiple)]
    public class JakeKnowsEngine : Logging, IJakeKnowsEngine
    {
        private static DataObjects _obj;
        private static FieldsDictionary _fields;
        private DataSet _data;
        private string _result;
        private int m_TaskID;
        private CancellationToken _CancellationToken;

        public static string ConnectionString;

        //public ConcurrentDictionary<string, int> _mTest = new ConcurrentDictionary<string, int>();

        public string Execute(DataSet data)
        {
            _CancellationToken = new CancellationToken();
            _data = data;
            LogDebug("JakeKnowsEngine Execute: this: " + this.GetHashCode().ToString() + " --- thread: " + Thread.CurrentThread.ManagedThreadId.ToString() + " sessionID: " + OperationContext.Current.SessionId.ToString());
            var task = Task<bool>.Factory.FromAsync(BeginDoWork, EndDoWork, _CancellationToken);
            //var task = Task<bool>.Factory.StartNew(() => WorkerFunction(_CancellationToken));
            LogDebug("this: " + this.GetHashCode().ToString() + " --- task: " + task.Id + " - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            task.Wait(_CancellationToken);
            //Thread.Sleep(2000);
            //LogDebug("JakeKnowsEngine Execute: this: " + this.GetHashCode().ToString() + " COMPLETED --- task: " + task.Id + " - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            m_TaskID = task.Id;
            _result += _data.GetXml();
            //LogDebug("JakeKnowsEngine Execute: this: " + this.GetHashCode().ToString() + " _result: " + _result);
            return _result;
        }
        public IAsyncResult BeginDoWork(AsyncCallback callback, object state)
        {
            var task = Task<bool>.Factory.StartNew(this.WorkerFunction, state);
            LogDebug("this: " + this.GetHashCode().ToString() + " --- task: " + task.Id + " - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            return task.ContinueWith(res => callback(task), TaskContinuationOptions.None);
        }
        public bool EndDoWork(IAsyncResult result)
        {
            try
            {
                bool res = ((Task<bool>)result).Result;
                LogDebug("this: " + this.GetHashCode().ToString() + " --- res: " + res + " - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
                return res;
            }
            catch (Exception e)
            {
                LogError("Exception " + e.Message);
            }
            return false;
        }

        private bool WorkerFunction(object tok)
        {
            CancellationToken token = (CancellationToken)tok;
            if (token.IsCancellationRequested)
            {
                // Terminate the operation. 
                return false;
            }
            //LogDebug("JakeKnowsEngine WorkerFunction");
            //LogDebug("JakeKnowsEngine WorkerFunction: this: " + this.GetHashCode().ToString() + " --- before  - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            int valid = ValidateInput.Validate(_data, _fields);
            valid = 0;
            //LogDebug("JakeKnowsEngine WorkerFunction: this: " + this.GetHashCode().ToString() + " --- after  - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            if (valid != 0)
            {
                LogWarn("Validation failed");
                _data.Tables.Remove("Parameters");
                _data.Tables.Remove("Function");
                _data.Tables.Remove("Payload");
                _data.AcceptChanges();
                return false;
            }
            bool res = ContextStrategy.Execute(new WSCall(_data), _obj);
            _data.Tables.Remove("Parameters");
            _data.Tables.Remove("Function");
            _data.Tables.Remove("Payload");
            _data.AcceptChanges();
            //LogDebug("JakeKnowsEngine WorkerFunction: this: " + this.GetHashCode().ToString() + " --- END  - thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
            return res;
        }

        public static void SetDataCache(DataObjects obj)
        {
            _obj = obj;
        }

        public static void SetFieldsDictionary(FieldsDictionary obj)
        {
            _fields = obj;
        }

        public static void Init()
        {
#if DEBUG
    LogInfo("DEBUG Version");
#else
    LogInfo("JakeKnowsEngine : RELEASE Version");
#endif
            LogInfo("Init");
            ConnectionString = ConfigurationManager.AppSettings["ConnectionString"];
            ValidateInput.Init();
            ContextStrategy.Init();
            //_mTest["Test"] = 1;
        }

        public string Test(string text)
        {
            LogInfo("Test --- SessionId " + OperationContext.Current.SessionId);
            //JakeKnowsEngine host = OperationContext.Current.Host as JakeKnowsEngine;
            
            //int id = _mTest["Tabula"];
            //string sID = id.ToString();
            //text += sID;
            text = Regex.Replace(text, " {2,}", " ");
            string s = "Hello " + text + "this: " + this.GetHashCode().ToString() + " - running in thread: '" + Thread.CurrentThread.ManagedThreadId.ToString() + "' hash: " + Thread.CurrentThread.GetHashCode();
            //string s = "Hello " + text + " --- counter: " + m_Counter + " - running in thread: " + Thread.CurrentThread.ManagedThreadId.ToString();
            return s;
        }
    }
}
