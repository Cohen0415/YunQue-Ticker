#ifndef PAGELIFECYCLEAWARE_H
#define PAGELIFECYCLEAWARE_H

class PageLifecycleAware {
public:
    virtual ~PageLifecycleAware() {}
    // 切换到本页
    virtual void onPageEnter() {}
    // 离开本页
    virtual void onPageLeave() {}
};

#endif // PAGELIFECYCLEAWARE_H
