# Operating System Tutorial from 0

 Start writing at 12 noon on February 10, 2025.

## Preface

 In October 2023, as a sophomore, I first came into contact with the operating system course. At that time, I had just roughly studied MIT's OS Lab (the experiment was not completed until the beginning of 2025, when I completed the work I should have done). At the same time, I also started my first difficult project, which was to create a runnable and simplest operating system from a blank!

 In fact, there are many excellent operating system tutorials, and I am very willing to give a few examples here!

> [SamyPesse/How-to-Make-a-Computer-Operating-System: How to Make a Computer Operating System in C++ (github.com)](https://github.com/SamyPesse/How-to-Make-a-Computer-Operating-System)
>
> [Cooi-Boi/Tiny-OS: "Restoring the True Image of the Operating System" self-written source code implementation and detailed record of the entire implementation process of the operating system on CSDN, including Debug steps and errors in the book Bochs2.6.8 Gcc4.4 This book implements all except the last three small functions. There are about 6k lines of code. I hope it can help you ^^ (github.com)](https://github.com/Cooi-Boi/Tiny-OS)

However, I noticed that the tool chains they used were too old. When I plucked up the courage to take a look at the tool chains they used, I found that they were basically outdated for more than ten years. For those who used the code of these tutorials without modification, either there were serial errors or they were stuck in the relay mode. For novices, they had no idea what was going on. They had to leave in frustration, or some people like me were willing to go back and mess with the old tool chains they used. It turns out that in the end, they would only mess up their environment. When I wrote this, I tested three groups of operating system environments.

- WSL (Windows Subsystem For Linux) (Arch WSL and Typical Ubuntu WSL have been tested),
- Ubuntu24.04,
- Arch Linux

 can correctly configure the target environment (Arch is a bit more difficult, we have to compile gcc4.4 and bochs with great effort, and the end result is a mess)

 Now as a preface, I will briefly explain what you need to do before starting:

1. Be patient, the author is also a novice in operating systems! I also encountered very frustrating problems in the previous hand-rubbing, but I persisted and finally spent a difficult month to eliminate the problems. They may be - writing the wrong configuration, copying the wrong code, putting our things in the wrong place, setting the wrong link order to cause illegal address access... Please, any one of them can give us a headache! However, I think these are a necessary test to become a qualified programmer - we need to develop an ability to solve the problems we encounter in an environment with limited information.
2. Learn to find information! It is meaningless if you don't work hard in isolation. What we need is to look up manuals, look up blogs written by others, compare other people's methods, and force ourselves to learn new technologies to solve the problems we encounter. Rather than completely copying other people's solutions. Taking "The Truth of Operating System Restoration" as an example, there are still many people who post their blogs almost completely original, **without any** thought, copying other people's output. This is not good! It does not help to improve your own abilities!
3. Try new tools. The author also read the documents of gcc and bochs, and read every communication carefully, hoping that when I click the next page or scroll my mouse wheel, I can finally see the answer I have been looking forward to. Well! It's still very difficult. When I was solving it, I basically stayed up all night until three o'clock every day, or got up at five in the morning to continue my research. Of course, you who read my blog here don't have to do so. It's better to treat it as a joke after tea and chat (laugh
4. Make sure your computer can run a virtual machine! The author must explain that because the content of the author's study is very classic and the level is even more chicken, **I can only write 32-bit operating systems instead of 64-bit**. There is not much time to really root in the tens of thousands of pages of `Intel/AMD` architecture manual to find answers page by page. So, I want to apologize for my slackness here! For this reason, the author uses the bochs virtual machine (note that it is not gdb debugging), which is relatively convenient for our debugging. At least, the Makefile given by the author is in debugging mode by default, which is convenient for us to control and observe the behavior and phenomena of the operating system. Of course, friends who are interested can naturally try to use bochs-with-gdb, but as of now, the author's work focus is still **to implement this operating system and complete as many functions as possible**, so please try it yourself if you have the spare time. If you are willing to share your work, send me Issue or PR (Pull Request) at any time, and I will check your work as soon as possible!

## So, I...

 Now, when you make up your mind, instead of backing off and closing the browser tab interface, then cheer yourself up and show that you are willing to take action and start your operating system hand-rolling journey!

 The author now gives these setup configurations, please complete your configuration work according to your needs! Here, the author's ramblings are finished! Good luck!

> Next: See the [setups](./setups/README.md)!
>
> Or english version: [setup](./setups/README_EN.md)