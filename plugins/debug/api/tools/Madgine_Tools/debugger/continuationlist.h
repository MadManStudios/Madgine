


namespace Engine {
namespace Tools {

    enum class ControlButton {
        NONE,
        PLAY,
        STEP,
        PAUSE,
        STOP
    };

    struct MADGINE_DEBUGGER_TOOLS_EXPORT ContinuationList {
        ContinuationList(ControlButton button);
        ContinuationList(const ContinuationList &) = delete;
        ~ContinuationList();

        ContinuationList &operator=(const ContinuationList &) = delete;

        void controls(Debug::Continuation &continuation);

    private:
        ControlButton mButton;
        std::vector<std::pair<Debug::Continuation, Debug::ContinuationMode>> mContinuations;
    };

}
}