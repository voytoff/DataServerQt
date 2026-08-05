Теперь применительно к нашему DataServer.

Ты сказал:

зависание - самая неприятная вещь для пользователя

И это действительно аргумент в пользу Guard.

Потому что ошибка:

Frame& frame = buffer.beginWrite();

processing(frame);

// забыли publish()

может остановить поток данных.

А с Guard:

{
    auto frame = buffer.writeFrame();

    processing(frame);
}

забыть невозможно.

Но я бы сделал аккуратно.

Не сразу "большую систему Guard".

А маленький первый вариант:

class FrameWriter
{
public:

    Frame& frame();

    void commit();

};

То есть сначала:

auto writer = buffer.beginWrite();

processing(writer.frame());

writer.commit();

А потом, когда появится реальная многопоточность, превращаем его в полноценный RAII:

{
    auto writer = buffer.writeFrame();

    processing(writer.frame());
}

Почему я бы не спешил?

Потому что сейчас у нас еще не определено:

может ли ProcessingGraph отказаться от кадра;
что делать при ошибке формулы;
что делать при неполных данных от DataSource;
может ли Archive задержать публикацию.

Это важные вопросы.

Но направление ты выбрал правильное.

Я бы зафиксировал архитектурное правило:

BufferManager не выдает произвольный Frame.
Изменяемый Frame получается только через write-путь.
Готовый Frame доступен только через read-путь.

То есть даже без Guard:

Frame& beginWrite();

void publish();

const Frame& readyFrame();

уже является хорошей защитой.

А Guard станет просто следующим уровнем защиты, когда появится многопоточность.

И еще один момент: для нашего проекта я бы скорее назвал его не Guard, а:

FrameWriter
FrameReader

Мне кажется, в DataServer это будет понятнее.

Когда через год откроешь код:

auto writer = buffer.beginWrite();

сразу понятно, что происходит.

А:

auto guard = buffer.acquire();

уже требует вспоминать детали.

Так что я бы сейчас заложил Writer/Reader как концепцию, но реализацию сделал простой. Это хорошо совпадает с нашим принципом: "не усложнять сейчас, но не закрывать дорогу вперед".


Позже без изменения архитектуры добавляем RAII

Сейчас:

auto& frame = buffer.beginWrite();

process(frame);

buffer.publish();

Позже:

auto writer = buffer.beginWrite();

process(writer.frame());

и:

~FrameWriter()
{
    buffer.publish();
}




Мне кажется Scheduler — это дирижер

Он сам ничего не вычисляет.

Он сам ничего не хранит.

Он сам не знает формулы.

Он просто запускает остальных.

Я вижу цикл примерно так.

Scheduler Tick
      │
      │
      ▼
beginWrite()

      │
      ▼
получить новый Frame

      │
      ▼
назначить

FrameNumber

Timestamp

WallClockTime

      │
      ▼
DataSource

      │
      ▼
ProcessingGraph

      │
      ▼
publish()

      │
      ▼
Archive

      │
      ▼
Publisher

      │
      ▼
следующий тик


Frame& frame = buffer.beginWrite();

frame.number = clock.nextFrame();

frame.timestamp = clock.now();

frame.wallTime = clock.wallNow();

dataSource.read(frame);

processing.process(frame);

buffer.publish();

archive.process(buffer.readFrame());

publisher.process(buffer.readFrame());