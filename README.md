# Простая USD/RUB биржа с клиент-серверной архитектурой

## Зависимости 
Для реализации использовалась библиотека boost.asio, для тестирования - GoogleTest

## Сборка проекта
Необходимо запустить скрипт $install.sh$. После его выполнения в созданной скриптом папке $build/bin$ будут лежать готовые исполняемые файлы.

---
## Описание
$Server$ - бинарник выполняющий роль сервера. При запуске можно указать численный аргумент - номер порта, на котором будет работать сервер. Запуск без параметра аналогичен команде ```./Server 5555```  
 
$Client$ - исполняемый файл клиента, при запуске можно указать адрес и порт сервера, запуск без параметров аналогичен ```./Client 127.0.0.1 5555```  

$tests$ - запускает тесты.

## Работа с клиентом
Доступно семь типов запросов, отображающихся в меню клиента, для выполнения запроса необходимо ввести в меню соответствующий номер.
- $Reg$ - зарегистрировать нового пользователя биржи. Возвращает его id
- $Auth$ - используя полученный при регистрации id войти в профиль. Если пользователя с введенным id не существует, или с ним уже есть открытая сессия; авторизация завершится неудачей. При успешном входе в профиль вы увидите уведомление (возможно пустое) о событиях, произошедших вне сессии.
- $Book$ - посмотреть котировки. Возвращает список открытых заказов в json формате.
- $Order$ - создание нового заказа. Необходимо указать сторону **Sell/Buy**, количество и цену. Вернет информацию о заказе, в том числе id
- $Cancel$ - отмена заказа по его id. В случае ввода чужого заказа завершится неудачей.
- $Info$ - получение информации о пользователе: USD, RUB счет, активные и закрытые заказы.
- $Exit$ - завершить текущую сессию
  
В момент совершения сделки оба участника получают уведомления об изменении своих заказов.  
'Response: null' означает пустой ответ сервера. (например в случае запроса $Book$ - на бирже нет ни одного заказа)
