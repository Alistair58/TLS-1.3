extern "C" {
#include "../src/args.h"
}
#include <gtest/gtest.h>


TEST(ArgsTest,helpClientValid){
    char *argv[] = {"program","-help"};
    testing::internal::CaptureStdout();
    Args res = parseArgsClient(2,argv); 
    ASSERT_EQ(res.option,DEALT_WITH);
    ASSERT_EQ(res.arg1,nullptr);
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "Usage:\n"
            "   -keygen -privpath=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates a key pair and stores the private and public keys at the specified file paths.\n"
            "   -certifgen -subject=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates an unsigned X509 certificate for this subject.\n"
            "       pubpath is the path of the subject's public key.\n"
            "   -connect -capubpath=\"{str}\"\n"
            "       Connect to a server using the public key of a CA found at capubpath.\n" 
            "       Once connected, prompts user for input, encrypts this and sends this to the server. Prints response from server. Loop repeats.\n"
            "   -help/--help"
            "       This.");
}

TEST(ArgsTest,keygenClientValid){
    char *argv[] = {"program","-keygen","-privpath=/priv/","-pubpath=/pub/"};
    Args res = parseArgsClient(4,argv); 
    ASSERT_EQ(res.option,KEY_GEN);
    ASSERT_STREQ(res.arg1,"/priv/");
    ASSERT_STREQ(res.arg2,"/pub/");
    ASSERT_EQ(res.arg3,nullptr);
}

TEST(ArgsTest,certifgenClientValid){
    char *argv[] = {"program","-certifgen","-subject=dave","-pubpath=/pub/"};
    Args res = parseArgsClient(4,argv); 
    ASSERT_EQ(res.option,CERTIF_GEN);
    ASSERT_STREQ(res.arg1,"dave");
    ASSERT_STREQ(res.arg2,"/pub/");
    ASSERT_EQ(res.arg3,nullptr);
}

TEST(ArgsTest,connectClientValid){
    char *argv[] = {"program","-connect","-capubpath={str}"};
    Args res = parseArgsClient(3,argv); 
    ASSERT_EQ(res.option,CONNECT);
    ASSERT_STREQ(res.arg1,"{str}");
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
}

TEST(ArgsTest,helpServerValid){
    char *argv[] = {"program","-help"};
    testing::internal::CaptureStdout();
    Args res = parseArgsServer(2,argv); 
    ASSERT_EQ(res.option,DEALT_WITH);
    ASSERT_EQ(res.arg1,nullptr);
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output, "Usage:\n"
            "   -keygen -privpath=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates a key pair and stores the private and public keys at the specified file paths.\n"
            "   -certifgen -subject=\"{str}\" -pubpath=\"{str}\"\n"
            "       Generates an unsigned X509 certificate for this subject.\n"
            "       pubpath is the path of the subject's public key.\n"
            "   -certifsign -certifpath=\"{str}\" -issuer=\"{str}\" -privpath=\"{str}\"\n"
            "       Signs an X509 certificate using the private key specified by the path and with the issuer's name on the certificate.\n"
            "       certifpath is the source file which will be overwritten with the signed version.\n"
            "   -connect\n"
            "       Listen to a socket and wait for a client.\n" 
            "       Once connected, prompts user for input, encrypts this and sends this to the client. Prints response from client. Loop repeats.\n"
            "   -help/--help"
            "       This.");
}   

TEST(ArgsTest,keygenServerValid){
    char *argv[] = {"program","-keygen","-privpath=/priv/","-pubpath=/pub/"};
    Args res = parseArgsServer(4,argv); 
    ASSERT_EQ(res.option,KEY_GEN);
    ASSERT_STREQ(res.arg1,"/priv/");
    ASSERT_STREQ(res.arg2,"/pub/");
    ASSERT_EQ(res.arg3,nullptr);
}

TEST(ArgsTest,certifgenServerValid){
    char *argv[] = {"program","-certifgen","-subject=dave","-pubpath=/pub/"};
    Args res = parseArgsServer(4,argv); 
    ASSERT_EQ(res.option,CERTIF_GEN);
    ASSERT_STREQ(res.arg1,"dave");
    ASSERT_STREQ(res.arg2,"/pub/");
    ASSERT_EQ(res.arg3,nullptr);
}   

TEST(ArgsTest,certifsignServerValid){
    char *argv[] = {"program","-certifsign","-certifpath=path","-issuer=dave","-privpath=/priv"};
    Args res = parseArgsServer(5,argv); 
    ASSERT_EQ(res.option,CERTIF_SIGN);
    ASSERT_STREQ(res.arg1,"path");
    ASSERT_STREQ(res.arg2,"dave");
    ASSERT_STREQ(res.arg3,"/priv");
}

TEST(ArgsTest,connectServerValid){
    char *argv[] = {"program","-connect"};
    Args res = parseArgsServer(2,argv); 
    ASSERT_EQ(res.option,CONNECT);
    ASSERT_EQ(res.arg1,nullptr);
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
}

TEST(ArgsTest,clientInvalid){
    char *argv[] = {"program","-invalidOption"};
    testing::internal::CaptureStdout();
    Args res = parseArgsClient(2,argv);
    std::string output = testing::internal::GetCapturedStdout(); 
    ASSERT_EQ(res.option,DEALT_WITH);
    ASSERT_EQ(res.arg1,nullptr);
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
    ASSERT_EQ(output,"Invalid option. Run -help for info on valid options.\n");
}

TEST(ArgsTest,serverInvalid){
    char *argv[] = {"program","-invalidOption"};
    testing::internal::CaptureStdout();    
    Args res = parseArgsServer(2,argv); 
    std::string output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(res.option,DEALT_WITH);
    ASSERT_EQ(res.arg1,nullptr);
    ASSERT_EQ(res.arg2,nullptr);
    ASSERT_EQ(res.arg3,nullptr);
    ASSERT_EQ(output,"Invalid option. Run -help for info on valid options.\n");
}

