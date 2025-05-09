//
// Created by GaGa on 25-5-8.
//

#include "DexDump.h"

#include "parser/HeaderParser.h"
#include "parser/StringPoolParser.h"
#include "parser/TypeParser.h"
#include "parser/ProtoParser.h"
#include "parser/FieldParser.h"
#include "parser/MethodParser.h"
#include "parser/ClassDefsParser.h"
#include "parser/CodeParser.h"

namespace dex
{
    DexDump::DexDump() : fileData_(nullptr), fileSize_(0)
    {
        // 初始化时不需要额外操作，DexContext会自动初始化
    }

    DexDump::~DexDump()
    {
        // 清理资源
        close();
    }

    bool DexDump::open(const char* fileName)
    {
        close();
        if (fileName == nullptr)
        {
            LOGE("文件名称不能为空");
            return false;
        }

        // 映射文件
        if (const int32_t result = util::mapFile(fileName, &fileData_, &fileSize_); result != 0)
        {
            LOGE("映射文件失败: %d", result);
            return false;
        }
        LOGI("成功打开DEX文件: %s", fileName);

        // 设置全局上下文数据
        DexContext& context = DexContext::getInstance();
        context.reset(); // 重置上下文，以防之前有残留数据
        context.setFileData(fileData_, fileSize_);

        // 开始解析文件
        parser();


        return true;
    }

    bool DexDump::parser()
    {
        // 解析头部信息
        if (!parseHeader())
        {
            LOGE("解析DEX头部失败");
            close();
            return false;
        }

        // 解析String信息
        if (!parseString())
        {
            LOGE("解析StringIds失败");
            close();
            return false;
        }

        // 解析Type
        if (!parseType())
        {
            LOGE("解析Type失败");
            close();
            return false;
        }

        if (!parseProto())
        {
            LOGE("解析Proto失败");
            close();
            return false;
        }

        // 解析Field
        if (!parseField())
        {
            LOGE("解析Field ID表失败");
            close();
            return false;
        }
        
        // 解析Method
        if (!parseMethod())
        {
            LOGE("解析Method ID表失败");
            close();
            return false;
        }
        
        // 解析ClassDef
        if (!parseClassDef())
        {
            LOGE("解析ClassDef表失败");
            close();
            return false;
        }

        LOGI("Dex解析完毕");
        return true;
    }

    bool DexDump::parseHeader()
    {
        DexContext& context = DexContext::getInstance();

        // 解析DEX头部
        parser::HeaderParser header_parser(context.getFileData(), context.getFileSize());

        // 调用parse方法执行实际解析
        if (!header_parser.parse())
        {
            LOGE("解析DEX头部失败: %s", header_parser.getLastError().c_str());
            return false;
        }

        // 保存解析结果到全局上下文
        context.setHeader(header_parser.getHeader());
        context.setValid(true);

        return true;
    }

    bool DexDump::parseString()
    {
        DexContext& context = DexContext::getInstance();

        // 解析StringIds
        parser::StringPoolParser string_parser(context.getFileData(), context.getFileSize());
        if (!string_parser.parse())
        {
            LOGE("解析StringIds失败: %s", string_parser.getLastError().c_str());
            return false;
        }

        // 加载所有字符串到内存
        if (!context.loadAllStrings())
        {
            LOGW("加载字符串内容失败，但继续解析");
        }

        context.setValid(true);
        return true;
    }

    bool DexDump::parseType()
    {
        DexContext& context = DexContext::getInstance();

        // 解析TypeIds
        parser::TypeParser type_parser(context.getFileData(), context.getFileSize());
        if (!type_parser.parse())
        {
            LOGE("解析TypeIds失败: %s", type_parser.getLastError().c_str());
            return false;
        }

        // 加载所有类型到内存
        if (!context.loadStringType())
        {
            LOGW("加载类型内容失败，但继续解析");
        }

        context.setValid(true);
        return true;
    }

    bool DexDump::parseProto()
    {
        DexContext& context = DexContext::getInstance();

        // 解析Proto ID表
        parser::ProtoParser proto_parser(context.getFileData(), context.getFileSize());
        if (!proto_parser.parse())
        {
            LOGE("解析Proto ID表失败: %s", proto_parser.getLastError().c_str());
            return false;
        }

        // 加载所有Proto信息到内存
        if (!context.loadAllProtos())
        {
            LOGW("加载Proto信息失败，但继续解析");
        }

        return true;
    }

    bool DexDump::parseField()
    {
        DexContext& context = DexContext::getInstance();

        // 解析Field ID表
        parser::FieldParser field_parser(context.getFileData(), context.getFileSize());
        if (!field_parser.parse())
        {
            LOGE("解析Field ID表失败: %s", field_parser.getLastError().c_str());
            return false;
        }

        // 加载所有Field信息到内存
        if (!context.loadAllFields())
        {
            LOGW("加载Field信息失败，但继续解析");
        }

        return true;
    }
    
    bool DexDump::parseMethod()
    {
        DexContext& context = DexContext::getInstance();

        // 解析Method ID表
        parser::MethodParser method_parser(context.getFileData(), context.getFileSize());
        if (!method_parser.parse())
        {
            LOGE("解析Method ID表失败: %s", method_parser.getLastError().c_str());
            return false;
        }

        // 加载所有Method信息到内存
        if (!context.loadAllMethods())
        {
            LOGW("加载Method信息失败，但继续解析");
        }

        return true;
    }
    
    bool DexDump::parseClassDef()
    {
        DexContext& context = DexContext::getInstance();

        // 解析ClassDef表
        parser::ClassDefsParser classDef_parser(context.getFileData(), context.getFileSize());
        if (!classDef_parser.parse())
        {
            LOGE("解析ClassDef表失败: %s", classDef_parser.getLastError().c_str());
            return false;
        }

        // 加载所有ClassDef信息到内存
        if (!context.loadAllClassDefs())
        {
            LOGW("加载ClassDef信息失败，但继续解析");
        }

        return true;
    }
    
    bool DexDump::parseCode(uint32_t methodIdx)
    {
        DexContext& context = DexContext::getInstance();
        
        // 检查方法索引是否有效
        if (methodIdx >= context.getMethodIdsCount())
        {
            LOGE("方法索引无效: %u", methodIdx);
            return false;
        }
        
        // 首先需要找到该方法对应的代码偏移量
        uint32_t codeOffset = 0;
        bool foundCode = false;
        
        // 遍历所有类定义
        for (uint32_t i = 0; i < context.getClassDefsCount(); i++)
        {
            const ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            // 检查是否有类数据
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                // 查找直接方法
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.methodIdx == methodIdx && method.codeOff != 0)
                    {
                        codeOffset = method.codeOff;
                        foundCode = true;
                        break;
                    }
                }
                
                // 如果在直接方法中没找到，查找虚拟方法
                if (!foundCode)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.methodIdx == methodIdx && method.codeOff != 0)
                        {
                            codeOffset = method.codeOff;
                            foundCode = true;
                            break;
                        }
                    }
                }
                
                if (foundCode)
                {
                    break;
                }
            }
        }
        
        if (!foundCode || codeOffset == 0)
        {
            LOGE("未找到方法 %u 的代码段", methodIdx);
            return false;
        }
        
        // 解析代码段
        return parseCodeByOffset(codeOffset);
    }
    
    bool DexDump::parseCodeByOffset(uint32_t codeOffset)
    {
        DexContext& context = DexContext::getInstance();
        
        // 检查偏移量是否有效
        if (codeOffset == 0 || codeOffset >= context.getFileSize())
        {
            LOGE("代码偏移量无效: 0x%08X", codeOffset);
            return false;
        }
        
        // 创建代码解析器
        parser::CodeParser codeParser(context.getFileData(), context.getFileSize());
        
        // 解析代码
        codeParser.parseCode(codeOffset);
        
        return true;
    }

    bool DexDump::parseDebugInfo(uint32_t methodIdx)
    {
        DexContext& context = DexContext::getInstance();
        
        // 确保代码信息已解析
        if (!parseCode(methodIdx))
        {
            return false;
        }
        
        // 获取方法代码偏移量和调试信息偏移量
        uint32_t debugInfoOff = 0;
        
        // 遍历所有类定义
        for (uint32_t i = 0; i < context.getClassDefsCount(); i++)
        {
            const ClassDefInfo classInfo = context.getClassDefInfo(i);
            
            // 检查是否有类数据
            if (classInfo.classDataOff != 0 && classInfo.classData.isLoaded)
            {
                // 查找直接方法
                for (const auto& method : classInfo.classData.directMethods)
                {
                    if (method.methodIdx == methodIdx && method.codeOff != 0)
                    {
                        if (!method.codeInfo.isLoaded)
                        {
                            return false;  // 代码未正确加载
                        }
                        
                        debugInfoOff = method.codeInfo.debugInfoOff;
                        break;
                    }
                }
                
                // 如果在直接方法中没找到，查找虚拟方法
                if (debugInfoOff == 0)
                {
                    for (const auto& method : classInfo.classData.virtualMethods)
                    {
                        if (method.methodIdx == methodIdx && method.codeOff != 0)
                        {
                            if (!method.codeInfo.isLoaded)
                            {
                                return false;  // 代码未正确加载
                            }
                            
                            debugInfoOff = method.codeInfo.debugInfoOff;
                            break;
                        }
                    }
                }
                
                if (debugInfoOff != 0)
                {
                    break;
                }
            }
        }
        
        if (debugInfoOff == 0)
        {
            LOGE("未找到方法 %u 的调试信息", methodIdx);
            return false;
        }
        
        // 解析调试信息
        return parseDebugInfoByOffset(debugInfoOff);
    }
    
    bool DexDump::parseDebugInfoByOffset(uint32_t debugInfoOffset)
    {
        DexContext& context = DexContext::getInstance();
        
        // 创建调试信息数据结构
        dex::DebugInfoData debugInfo;
        
        // 解析调试信息
        if (!context.parseDebugInfo(debugInfoOffset, debugInfo))
        {
            LOGE("解析调试信息失败: 0x%08X", debugInfoOffset);
            return false;
        }
        
        return true;
    }

    void DexDump::close()
    {
        if (fileData_ != nullptr)
        {
            // 解除映射
            util::unmapFile(fileData_);
            fileData_ = nullptr;
            fileSize_ = 0;

            // 重置全局上下文
            DexContext::getInstance().reset();
        }
    }

    bool DexDump::isValid()
    {
        return DexContext::getInstance().isValid();
    }

    DexContext& DexDump::getContext()
    {
        return DexContext::getInstance();
    }
}
