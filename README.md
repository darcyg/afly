# Alibaba feiyan cloud:  
	This is a gateway demo which connects to alibaba feiyan cloud and runs on SOC mtk7620.  
	  
	This Demo Bridages Messages Between NXP and Alibaba Cloud .  

# Author:   
	dyx/tech32@hzdusun.com  
	au/ghaucience@gmail.com

# Use:   
	1.  
	2. make  

# !!!:  
	...  

# Developing:  
	1. product_sub & product module   [ok ]  
	3. ubus proto -> afly module [ok ]
	3. device reg->create->login->unlogin->destroy->unreg [ok ]
	4. dev sevice framework  add pass/del pass/storage pass  [ok]
	5. report [ok]
	6. sdk watch [...]
	7. add product & del product link debug [ok ]  
	8. 网关更新nxp程序,基线和旧版本不一样
	9. json及内存泄露问题 [待查, 不一定有]
	10. 增加回连 [ok ]
	11. report 根据类型查找 [...]
	12. 属性跟新的时候才上报或者才保存
	13. cmd.c 当输入为是前缀空格的时候的 bug [ok]
	14. 增加subdev的时候出发list上报 [ok]
	15. 网关的默认模版参数上报, 解析获取默认参数 [ok]
	16. 增加网关程序版本上报 [ok]
	17. 增加动态密码和次数密码 [OK] 
	18. 合并到Dusun-Openwrit 系统包 [OK]
	19. 一键开门 [OK]
	20. 支持量产功能及静态增加Key&Secert功能 [ok]
	21. OTA
	22. Cloud OTA Get获取

	
# Version:  
	V1.0.0_afly_beta0:  
		- Functions:  
			*  公寓密码锁对接(支持普通密码/次数密码/动态密码/一键开门,不支持次数密码和动态密码)
		- Changes:  
			*  
		- Problem:  
			*  
		- Todo:  
			*  

# Lays 
	
					-AFly-
	---------------------------------------
	-Z3- -nxp-   -storage-
	-----------
	 -uproto-

