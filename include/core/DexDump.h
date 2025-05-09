//
// Created by GaGa on 25-5-8.
//

#ifndef DEXDUMP_H
#define DEXDUMP_H

#include <cstdint>

#include "DexFile.h"
#include "util.h"
#include "DexContext.h"
#include "log/log.h"

namespace dex
{
    /**
     * DexDump - 主控制类
     * 负责管理DEX文件解析和格式化的整体流程
     * 作为解析器和格式化器的桥梁
     */
    class DexDump
    {
    private:
        // 映射的文件数据（由DexDump管理生命周期）
        uint8_t* fileData_;
        
        // 文件大小
        size_t fileSize_;

    public:
        DexDump();
        ~DexDump();

        // 打开DEX文件
        bool open(const char* fileName);

        // 关闭DEX文件
        void close();

        // 总体解析器控制
        bool parser();

        // 检查DEX文件是否有效
        [[nodiscard]] static bool isValid();
        
        // 解析DEX头部
        static bool parseHeader();

        // 解析String信息
        static bool parseString();

        // 解析Type
        static bool parseType();

        // 解析Proto
        static bool parseProto();

        // 解析Field信息
        static bool parseField();
        
        // 解析Method信息
        static bool parseMethod();
        
        // 解析ClassDef信息
        static bool parseClassDef();
        
        // 解析Code信息
        static bool parseCode(uint32_t methodIdx);
        static bool parseCodeByOffset(uint32_t codeOffset);
        
        // 解析方法调试信息
        static bool parseDebugInfo(uint32_t methodIdx);
        static bool parseDebugInfoByOffset(uint32_t debugInfoOffset);

        // 获取全局上下文
        static DexContext& getContext();
    };
}

#endif //DEXDUMP_H
