// Problem 调试:
    Problem a;
    int acount = 0, fcount = 0;
    int pnt = 0;
    while (1) {
        int opcode;
        /*
        Test
        0 -> 封榜
        1 -> 解封 + 解冻
        2 -> 交题，提供：状态 时间
        3 -> 查所有提交
        4 -> 查状态提交，提供：状态
        
        只有0是通过哦
        */
        std::cin >> opcode;
        int stt, tmm;
        switch(opcode) {
            case 0:
                is_freezed = true; // ！？封封？！
                std::printf("排行榜已冻结\n");
                break;
            case 1:
                is_freezed = false;
                pnt = a.Release(acount, fcount);
                std::printf("排行榜已解冻\n");
                break;
            case 2:
                std::cin >> stt >> tmm;
                std::cout << "提交成功\n";
                pnt = a.Submit(stt, tmm, acount, fcount);
                break;
            case 3:
                std::cout << "查询全部提交:";
                std::cout << a.QueryAllSubmision().first << " " << a.QueryAllSubmision().second << std::endl;
                break;
            case 4:
                std::cin >> stt;
                std::cout << "查询指定提交:";
                std::cout << a.QueryStatusSubmision(stt) << std::endl;
                break;
            default:
                return 0;
        }
        std::printf("罚时：%d %s %s\n状态字符串:", pnt, (acount ? "显示通过" : "显示不通过"), (fcount ? "题目冻结" : "题目未冻结"));
        std::cout << a.Status() << std::endl;
    }