//
// Created by GaGa on 25-5-8.
//

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "core/DexDump.h"
#include "formatter/FieldPrint.h"
#include "formatter/HeaderPrint.h"
#include "formatter/StringPrint.h"
#include "formatter/ProtoPrint.h"
#include "formatter/MethodPrint.h"
#include "formatter/ClassPrint.h"
#include "formatter/CodePrint.h"
#include "formatter/DebugInfoPrint.h"
#include "log/log.h"

// 测试选项
enum TestOption
{
    TEST_ALL = 0,        // 测试所有项
    TEST_HEADER = 1,     // 测试头部
    TEST_STRING = 2,     // 测试字符串表
    TEST_PROTO = 3,      // 测试原型表
    TEST_FIELD = 4,      // 测试字段表
    TEST_METHOD = 5,     // 测试方法表
    TEST_CLASS = 6,      // 测试类表
    TEST_CODE = 7,       // 测试代码
    TEST_METHOD_CODE = 8, // 测试特定方法的代码
    TEST_DEBUG_INFO = 9,  // 测试调试信息
    TEST_METHOD_DEBUG = 10 // 测试特定方法的调试信息
};

int main(int _argc, char* const _argv[])
{
    // 启用日志颜色
    log_enable_color(true);
    
    // 创建DexDump实例
    dex::DexDump dex_dump{};
    
    // 打开DEX文件
    if (!dex_dump.open("D:\\ProjectALL\\CLionProjects\\DexDump\\resources\\classes2.dex"))
    {
        LOGE("打开DEX文件失败");
        return 1;
    }
    
    // 设置测试选项
    TestOption option = TEST_STRING;
    
    // 所有解析器的数据都通过DexContext共享
    // 所有格式化器都从DexContext获取数据
    
    switch (option)
    {
        case TEST_ALL:
            // 测试所有功能
            // 打印头部信息
            {
                dex::print::HeaderPrint header_print{};
                header_print.print();
            }
            
            // 打印字符串表
            {
                dex::print::StringPrint string_print{};
                string_print.print();
            }
            
            // 打印Proto表
            {
                dex::print::ProtoPrint proto_print{};
                proto_print.print();
            }
            
            // 打印Field表
            {
                dex::print::FieldPrint field_print{};
                field_print.print();
            }
            
            // 打印Method表
            {
                dex::print::MethodPrint method_print{};
                method_print.print();
            }
            
            // 打印Class表
            {
                dex::print::ClassPrint class_print{};
                class_print.print();
            }
            
            // 打印代码表
            {
                dex::print::CodePrint code_print{};
                code_print.print();
            }
            break;
            
        case TEST_HEADER:
            // 打印头部信息
            {
                dex::print::HeaderPrint header_print{};
                header_print.print();
            }
            break;
            
        case TEST_STRING:
            // 打印字符串表
            {
                dex::print::StringPrint string_print{};
                string_print.print();
            }
            break;
            
        case TEST_PROTO:
            // 打印Proto表
            {
                dex::print::ProtoPrint proto_print{};
                proto_print.print();
            }
            break;
            
        case TEST_FIELD:
            // 打印Field表
            {
                dex::print::FieldPrint field_print{};
                field_print.print();
            }
            break;
            
        case TEST_METHOD:
            // 打印Method表
            {
                dex::print::MethodPrint method_print{};
                method_print.print();
            }
            break;
            
        case TEST_CLASS:
            // 打印Class表
            {
                dex::print::ClassPrint class_print{};
                class_print.print();
            }
            break;
            
        case TEST_CODE:
            // 打印代码概览
            {
                dex::print::CodePrint code_print{};
                code_print.print();
            }
            break;
            
        case TEST_METHOD_CODE:
            // 打印特定方法的代码
            {
                // 假设我们想查看方法ID为10的代码
                uint32_t methodIdx = 10;
                
                // 解析代码
                if (dex::DexDump::parseCode(methodIdx))
                {
                    dex::print::CodePrint code_print{};
                    code_print.printMethodCode(methodIdx);
                }
                else
                {
                    LOGE("解析方法 %u 的代码失败", methodIdx);
                }
            }
            break;
            
        case TEST_DEBUG_INFO:
            // 打印调试信息概览
            {
                dex::print::DebugInfoPrint debug_print{};
                debug_print.print();
            }
            break;
            
        case TEST_METHOD_DEBUG:
            // 打印特定方法的调试信息
            {
                // 假设我们想查看方法ID为10的调试信息
                uint32_t methodIdx = 10;
                
                // 解析调试信息
                if (dex::DexDump::parseDebugInfo(methodIdx))
                {
                    dex::print::DebugInfoPrint debug_print{};
                    debug_print.printMethodDebugInfo(methodIdx);
                }
                else
                {
                    LOGE("解析方法 %u 的调试信息失败", methodIdx);
                }
            }
            break;
    }
    
    // 关闭DEX文件
    dex_dump.close();
    
    return 0;
}
