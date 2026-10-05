#include "detector/YoloPeopleDetector.hpp"

#include <iostream>
#include <stdexcept>

YoloPeopleDetector::YoloPeopleDetector(const std::string& modelPath)
{
    net_ = cv::dnn::readNetFromONNX(modelPath);
    if(net_.empty())
    {
        throw std::runtime_error("Failed to load YOLO model from: " + modelPath);
    }

    std::cout
        << "YOLO model loaded successfully: "
        << modelPath
        << std::endl;
}

std::vector<cv::Rect>
YoloPeopleDetector::detect(const cv::Mat& frame)
{
    std::vector<cv::Rect> people;

    if(frame.empty())
    {
        return people;
    }

    // ==============================
    // Letterbox：等比例缩放 + 补边
    // ==============================

    // 计算宽、高分别允许的缩放比例，取较小的那个。
    // 这样可以保证整张图都塞进 640×640，并且不变形。
    float scale = std::min(
        static_cast<float>(inputWidth_) / frame.cols,
        static_cast<float>(inputHeight_) / frame.rows
    );

    // 等比例缩放后的尺寸
    int resizedWidth =
        static_cast<int>(frame.cols * scale);

    int resizedHeight =
        static_cast<int>(frame.rows * scale);


    // 先等比例缩放
    cv::Mat resizedFrame;

    cv::resize(
        frame,
        resizedFrame,
        cv::Size(resizedWidth, resizedHeight)
    );


    // 计算需要补多少边
    int padX = (inputWidth_ - resizedWidth) / 2;
    int padY = (inputHeight_ - resizedHeight) / 2;

    // 如果不能整除，把剩余像素补到右边/下边
    int padRight =
        inputWidth_ - resizedWidth - padX;

    int padBottom =
        inputHeight_ - resizedHeight - padY;


    // 最终的 640×640 Letterbox 图像
    cv::Mat letterboxFrame;

    cv::copyMakeBorder(
        resizedFrame,
        letterboxFrame,
        padY,       // 上
        padBottom,  // 下
        padX,       // 左
        padRight,   // 右
        cv::BORDER_CONSTANT,
        cv::Scalar(114, 114, 114) // YOLO 常用的补边颜色
    );


    // Letterbox 图像 -> 神经网络输入
    cv::Mat blob;

    cv::dnn::blobFromImage(
        letterboxFrame,
        blob,
        1.0 / 255.0,
        cv::Size(inputWidth_, inputHeight_),
        cv::Scalar(),
        true,   // BGR -> RGB
        false
    );

    net_.setInput(blob);

    net_.setInput(blob);


    // 2. YOLO 推理
    std::vector<cv::Mat> outputs;

    net_.forward(
        outputs,
        net_.getUnconnectedOutLayersNames()
    );

    if(outputs.empty())
    {
        return people;
    }


    // YOLOv8 输出：
    //
    // [1, 84, 8400]
    //
    // 我们转换成：
    //
    // [8400, 84]
    //
    // 这样每一行就是一个候选框。
    cv::Mat output = outputs[0];

    cv::Mat rawMat(
        output.size[1],      // 84
        output.size[2],      // 8400
        CV_32F,
        output.ptr<float>()
    );

    // 转置后的结果：8400 × 84
    cv::Mat detectionMat;
    cv::transpose(rawMat, detectionMat);


    // 保存通过置信度筛选的候选框
    std::vector<cv::Rect> boxes;
    std::vector<float> scores;




    // 3. 遍历 8400 个候选框
    for(int i = 0; i < detectionMat.rows; ++i)
    {
        float* data = detectionMat.ptr<float>(i);

        // 前四个数据
        float cx = data[0];
        float cy = data[1];
        float width = data[2];
        float height = data[3];

        // COCO 中 person 是第 0 类
        //
        // data[4] 就是 person 的置信度
        float personScore = data[4];


        // 置信度太低就不要
        if(personScore < confidenceThreshold_)
        {
            continue;
        }



        // YOLO 使用的是中心点坐标：
        //
        //      (cx,cy)
        //         +
        //    ┌─────────┐
        //    │         │
        //    └─────────┘
        //
        // cv::Rect 需要左上角坐标，所以转换一下。
        // YOLO 输出现在是 0~1 的归一化坐标。
        // 先恢复到 640×640 模型输入坐标。
        float modelCx = cx * inputWidth_;
        float modelCy = cy * inputHeight_;

        float modelWidth =
            width * inputWidth_;

        float modelHeight =
            height * inputHeight_;


        // 模型坐标 -> 左上角坐标
        float x1 =
            modelCx - modelWidth / 2.0f;

        float y1 =
            modelCy - modelHeight / 2.0f;


        // 去掉 Letterbox 补边，
        // 再除以缩放比例，恢复到原始摄像头图像坐标。
        x1 = (x1 - padX) / scale;
        y1 = (y1 - padY) / scale;

        float originalWidth =
            modelWidth / scale;

        float originalHeight =
            modelHeight / scale;


        int left =
            static_cast<int>(x1);

        int top =
            static_cast<int>(y1);

        int boxWidth =
            static_cast<int>(originalWidth);

        int boxHeight =
            static_cast<int>(originalHeight);


        // YOLO 计算出来的框
        cv::Rect box(
            left,
            top,
            boxWidth,
            boxHeight
        );

        // 原始图像范围
        cv::Rect imageArea(
            0,
            0,
            frame.cols,
            frame.rows
        );

        // 把检测框裁剪到图像范围内
        box = box & imageArea;

        // 防止产生无效框
        if(box.width <= 0 || box.height <= 0)
        {
            continue;
        }

        // 保存有效候选框
        boxes.push_back(box);
        scores.push_back(personScore);
    }


    // 4. NMS 去除重复框
    std::vector<int> indices;

    cv::dnn::NMSBoxes(
        boxes,
        scores,
        confidenceThreshold_,
        nmsThreshold_,
        indices
    );


    // 5. 只保留 NMS 后最终的人框
    for(int index : indices)
    {
        const cv::Rect& box = boxes[index];
        people.push_back(box);
    }


    return people;
}