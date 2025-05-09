//
// Created by GaGa on 25-5-8.
//

#include "HeaderParser.h"

#include "log/log.h"

namespace dex::parser
{
    /**
     * 构建方法只负责传递数据。
     * 将fileData和fileSize赋值给父类的构建函数
     * @param fileData 文件数据
     * @param fileSize 数据大小
     */
    HeaderParser::HeaderParser(const uint8_t* fileData, const size_t fileSize): BaseParser(fileData, fileSize, nullptr),
                                                                                isValid_(false)
    {

        if (fileData == nullptr || fileSize == 0)
        {
            setError("文件数据指针为空");
            return;
        }
    }

    bool HeaderParser::parse()
    {
        // 检查文件数据是否有效
        if (!BaseFileData_)
        {
            setError("文件数据为空，无法解析");
            return false;
        }

        // 从文件数据复制头部结构
        memcpy(&header_, BaseFileData_, sizeof(DexHeader));
        // 验证
        if (!validateMagic())
        {
            return false;
        }
        if (!validateFileSize())
        {
            LOGW("文件大小不匹配，但继续解析");
        }
        if (!validateHeaderSize())
        {
            return false;
        }
        if (!validateEndianTag())
        {
            return false;
        }

        if (!validateSectionOffsets())
        {
            return false;
        }

        // 所有验证通过
        isValid_ = true;
        header_ = getHeader();
        LOGI("DEX文件头部解析成功");

        return true;
    }

    const DexHeader& HeaderParser::getHeader() const
    {
        return header_;
    }

    bool HeaderParser::isValid(const uint8_t* fileData)
    {
        if (fileData == nullptr)
        {
            return false;
        }

        // 检查魔数是否为"dex\n"
        if (memcmp(fileData, "dex\n", 4) != 0)
        {
            return false;
        }

        return true;
    }

    bool HeaderParser::validateMagic() const
    {
        // 检查魔数是否为"dex\n"
        if (memcmp(header_.magic, "dex\n", 4) != 0)
        {
            setError("无效的DEX文件魔数");
            return false;
        }

        // 检查版本号
        const char* version = reinterpret_cast<const char*>(header_.magic + 4);
        if (memcmp(version, "035", 3) != 0 &&
            memcmp(version, "037", 3) != 0 &&
            memcmp(version, "038", 3) != 0 &&
            memcmp(version, "039", 3) != 0)
        {
            LOGW("未知的DEX版本: %.3s", version);
            // 未知版本仅警告，不失败
        }

        return true;
    }

    bool HeaderParser::validateFileSize() const
    {
        // 检查文件大小是否与头部声明一致
        if (header_.fileSize != BaseFileSize_)
        {
            LOGW("文件大小不匹配: 头部声明 %u, 实际 %zu", header_.fileSize, BaseFileSize_);
            return false;
        }

        return true;
    }

    bool HeaderParser::validateHeaderSize() const
    {
        // 头部大小必须至少是DexHeader结构的大小
        if (header_.headerSize < sizeof(DexHeader))
        {
            setError("头部大小无效");
            return false;
        }

        return true;
    }

    bool HeaderParser::validateEndianTag() const
    {
        // DEX文件使用小端格式，大端标记应为0x12345678
        static constexpr uint32_t ENDIAN_CONSTANT = 0x12345678;
        if (header_.endianTag != ENDIAN_CONSTANT)
        {
            setError("不支持的字节序: 0x%08X", header_.endianTag);
            return false;
        }

        return true;
    }

    bool HeaderParser::validateSectionOffsets() const
    {
        // 检查各个段的偏移量是否在文件范围内

        // 检查字符串ID表
        if (header_.stringIdsSize > 0)
        {
            if (!isValidOffset(header_.stringIdsOff, header_.stringIdsSize * 4))
            {
                setError("字符串ID表偏移量无效");
                return false;
            }
        }

        // 检查类型ID表
        if (header_.typeIdsSize > 0)
        {
            if (!isValidOffset(header_.typeIdsOff, header_.typeIdsSize * 4))
            {
                setError("类型ID表偏移量无效");
                return false;
            }
        }

        // 检查原型ID表
        if (header_.protoIdsSize > 0)
        {
            if (!isValidOffset(header_.protoIdsOff, header_.protoIdsSize * 12))
            {
                setError("原型ID表偏移量无效");
                return false;
            }
        }

        // 检查字段ID表
        if (header_.fieldIdsSize > 0)
        {
            if (!isValidOffset(header_.fieldIdsOff, header_.fieldIdsSize * 8))
            {
                setError("字段ID表偏移量无效");
                return false;
            }
        }

        // 检查方法ID表
        if (header_.methodIdsSize > 0)
        {
            if (!isValidOffset(header_.methodIdsOff, header_.methodIdsSize * 8))
            {
                setError("方法ID表偏移量无效");
                return false;
            }
        }

        // 检查类定义表
        if (header_.classDefsSize > 0)
        {
            if (!isValidOffset(header_.classDefsOff, header_.classDefsSize * 32))
            {
                setError("类定义表偏移量无效");
                // setError("类定义表偏移量无效");
                return false;
            }
        }

        // 检查数据段
        if (header_.dataSize > 0)
        {
            if (!isValidOffset(header_.dataOff, header_.dataSize))
            {
                setError("数据段偏移量无效");
                return false;
            }
        }

        return true;
    }
}
